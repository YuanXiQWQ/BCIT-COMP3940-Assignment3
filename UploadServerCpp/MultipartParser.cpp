#include "MultipartParser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace{
    std::string trim(const std::string &value){
        std::size_t start = 0;
        while(start < value.size() &&
              std::isspace(static_cast<unsigned char>(value[start]))){
            ++ start;
        }
        std::size_t end = value.size();
        while(end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))){
            -- end;
        }
        return value.substr(start, end - start);
    }
}

bool MultipartParser::parse(HttpServletRequest &request){
    auto contentTypeOpt = request.getHeader("Content-Type");
    if(! contentTypeOpt.has_value()){
        return false;
    }

    if(contentTypeOpt->find("multipart/form-data") == std::string::npos){
        return false;
    }

    std::string boundary = extractBoundary(*contentTypeOpt);
    if(boundary.empty()){
        return false;
    }

    const auto &body = request.getBody();
    auto parts = splitParts(body, boundary);
    if(parts.empty()){
        return false;
    }

    bool parsed = false;
    for(auto &part: parts){
        auto cdIt = part.headers.find("content-disposition");
        if(cdIt == part.headers.end()){
            continue;
        }

        auto dispositionAttrs = parseContentDisposition(cdIt->second);
        auto nameIt = dispositionAttrs.find("name");
        if(nameIt == dispositionAttrs.end()){
            continue;
        }

        std::string fieldName = nameIt->second;
        auto filenameIt = dispositionAttrs.find("filename");

        if(filenameIt != dispositionAttrs.end() && ! filenameIt->second.empty()){
            UploadedFile file;
            file.fieldName = fieldName;
            file.originalFileName = filenameIt->second;

            auto contentTypeIter = part.headers.find("content-type");
            if(contentTypeIter != part.headers.end()){
                file.contentType = contentTypeIter->second;
            } else{
                file.contentType = "application/octet-stream";
            }
            file.data = std::move(part.data);
            request.setUploadedFile(std::move(file));
            parsed = true;
        } else{
            std::string value(part.data.begin(), part.data.end());
            while(! value.empty() && (value.back() == '\r' || value.back() == '\n')){
                value.pop_back();
            }
            request.setParameter(fieldName, value);
            parsed = true;
        }
    }

    return parsed;
}

std::string MultipartParser::extractBoundary(const std::string &contentType){
    auto boundaryPos = contentType.find("boundary=");
    if(boundaryPos == std::string::npos){
        return {};
    }

    std::string boundary = contentType.substr(boundaryPos + 9);
    if(! boundary.empty() && boundary.front() == '"' && boundary.back() == '"'){
        boundary = boundary.substr(1, boundary.size() - 2);
    }
    return boundary;
}

std::vector<MultipartParser::Part>
MultipartParser::splitParts(const std::vector<char> &body, const std::string &boundary){
    std::vector<Part> parts;
    if(body.empty()){
        return parts;
    }

    std::string buffer(body.begin(), body.end());
    std::string delimiter = "--" + boundary;
    std::string closingDelimiter = delimiter + "--";

    std::size_t position = 0;
    while(true){
        std::size_t boundaryPos = buffer.find(delimiter, position);
        if(boundaryPos == std::string::npos){
            break;
        }

        std::size_t start = boundaryPos + delimiter.size();
        if(buffer.compare(start, 2, "--") == 0){
            break; // reached closing boundary
        }

        if(buffer.compare(start, 2, "\r\n") == 0){
            start += 2;
        }

        std::size_t headerEnd = buffer.find("\r\n\r\n", start);
        if(headerEnd == std::string::npos){
            break;
        }

        std::string headerBlock = buffer.substr(start, headerEnd - start);
        auto headers = parsePartHeaders(headerBlock);

        std::size_t dataStart = headerEnd + 4; // skip CRLF CRLF
        std::size_t nextBoundary = buffer.find(delimiter, dataStart);
        if(nextBoundary == std::string::npos){
            break;
        }

        std::size_t dataEnd = nextBoundary;
        if(dataEnd >= 2 && buffer.compare(dataEnd - 2, 2, "\r\n") == 0){
            dataEnd -= 2;
        }

        Part part;
        part.headers = std::move(headers);
        part.data.assign(body.begin() + static_cast<long>(dataStart),
                         body.begin() + static_cast<long>(dataEnd));
        parts.emplace_back(std::move(part));

        position = nextBoundary;
        if(buffer.compare(nextBoundary, closingDelimiter.size(), closingDelimiter) == 0){
            break;
        }
    }

    return parts;
}

std::unordered_map<std::string, std::string>
MultipartParser::parsePartHeaders(const std::string &headerBlock){
    std::unordered_map<std::string, std::string> headers;
    std::istringstream stream(headerBlock);
    std::string line;
    while(std::getline(stream, line)){
        if(! line.empty() && line.back() == '\r'){
            line.pop_back();
        }
        auto colon = line.find(':');
        if(colon == std::string::npos){
            continue;
        }
        std::string name = normalizeHeaderName(trim(line.substr(0, colon)));
        std::string value = trim(line.substr(colon + 1));
        headers[name] = value;
    }
    return headers;
}

std::string MultipartParser::normalizeHeaderName(const std::string &name){
    std::string lowered = name;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch){
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

std::unordered_map<std::string, std::string>
MultipartParser::parseContentDisposition(const std::string &headerValue){
    std::unordered_map<std::string, std::string> attributes;
    std::string value = headerValue;

    std::size_t semicolon = value.find(';');
    if(semicolon == std::string::npos){
        return attributes;
    }

    std::size_t position = semicolon + 1;
    while(position < value.size()){
        std::size_t eq = value.find('=', position);
        if(eq == std::string::npos){
            break;
        }

        std::string key = trim(value.substr(position, eq - position));
        position = eq + 1;

        std::string rawValue;
        if(position < value.size() && value[position] == '"'){
            ++ position;
            std::size_t endQuote = value.find('"', position);
            if(endQuote == std::string::npos){
                break;
            }
            rawValue = value.substr(position, endQuote - position);
            position = endQuote + 1;
        } else{
            std::size_t nextSemicolon = value.find(';', position);
            if(nextSemicolon == std::string::npos){
                rawValue = value.substr(position);
                position = value.size();
            } else{
                rawValue = value.substr(position, nextSemicolon - position);
                position = nextSemicolon;
            }
        }

        attributes[normalizeHeaderName(key)] = rawValue;

        std::size_t next = value.find(';', position);
        if(next == std::string::npos){
            break;
        }
        position = next + 1;
    }

    return attributes;
}
