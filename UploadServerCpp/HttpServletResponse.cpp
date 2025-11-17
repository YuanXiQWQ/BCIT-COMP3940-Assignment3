#include "HttpServletResponse.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace{
    std::string normalizeHeaderName(const std::string &raw){
        std::string lowered = raw;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                       [](unsigned char ch){
                           return static_cast<char>(std::tolower(ch));
                       });
        return lowered;
    }

    std::string formatHeaderName(const std::string &normalized){
        std::string formatted;
        formatted.reserve(normalized.size());
        bool capitalize = true;
        for(char ch: normalized){
            if(ch == '-'){
                formatted.push_back('-');
                capitalize = true;
                continue;
            }
            if(capitalize){
                formatted.push_back(
                        static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
                capitalize = false;
            } else{
                formatted.push_back(
                        static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
        }
        return formatted;
    }
}

HttpServletResponse::HttpServletResponse()
        : statusCode(200), statusMessage("OK"), headers(), body(){
    setHeader("Connection", "close");
    setContentType("text/html; charset=utf-8");
}

void HttpServletResponse::setStatus(int code, std::string message){
    statusCode = code;
    statusMessage = std::move(message);
}

void HttpServletResponse::setHeader(const std::string &name, const std::string &value){
    headers[normalizeHeaderName(name)] = value;
}

void HttpServletResponse::setContentType(const std::string &value){
    setHeader("Content-Type", value);
}

void HttpServletResponse::write(const std::string &text){
    body.insert(body.end(), text.begin(), text.end());
}

void HttpServletResponse::writeBinary(const std::vector<char> &data){
    body.insert(body.end(), data.begin(), data.end());
}

void HttpServletResponse::writeBinary(const char *data, std::size_t length){
    body.insert(body.end(), data, data + length);
}

std::vector<char> HttpServletResponse::buildResponse() const{
    std::unordered_map<std::string, std::string> allHeaders = headers;
    allHeaders[normalizeHeaderName("Content-Length")] = std::to_string(body.size());

    std::ostringstream head;
    head << "HTTP/1.1 " << statusCode << ' ' << statusMessage << "\r\n";
    for(const auto &entry: allHeaders){
        head << formatHeaderName(entry.first) << ": " << entry.second << "\r\n";
    }
    head << "\r\n";

    std::string headerStr = head.str();
    std::vector<char> response;
    response.reserve(headerStr.size() + body.size());
    response.insert(response.end(), headerStr.begin(), headerStr.end());
    response.insert(response.end(), body.begin(), body.end());
    return response;
}
