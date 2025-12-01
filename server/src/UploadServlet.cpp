#include "UploadServlet.hpp"

#include "HttpServletResponse.hpp"
#include "UploadDirectoryManager.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace {
struct FormPart {
    std::string name;
    std::string filename;
    std::map<std::string, std::string> headers;
    std::vector<std::uint8_t> data;

    std::string asText() const
    {
        return std::string(reinterpret_cast<const char*>(data.data()), data.size());
    }
};

std::string escapeHtml(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for(char ch : input)
    {
        switch(ch)
        {
            case '&': out.append("&amp;"); break;
            case '<': out.append("&lt;"); break;
            case '>': out.append("&gt;"); break;
            case '"': out.append("&quot;"); break;
            case '\'': out.append("&#39;"); break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

std::string loadFormHtml()
{
    std::ifstream file("Form.html", std::ios::binary);
    if(file)
    {
        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }
    return "<!DOCTYPE html>\r\n"
           "<html lang=\"en\">\r\n"
           "<head>\r\n"
           "    <meta charset=\"UTF-8\"/>\r\n"
           "    <title>Upload</title>\r\n"
           "</head>\r\n"
           "<body>\r\n"
           "<h2>File Upload</h2>\r\n"
           "<form method=\"POST\" action=\"/\" enctype=\"multipart/form-data\">\r\n"
           "    Caption: <input type=\"text\" name=\"caption\"/><br/><br/>\r\n"
           "    Date: <input type=\"date\" name=\"date\"/><br/><br/>\r\n"
           "    File: <input type=\"file\" name=\"fileName\"/><br/><br/>\r\n"
           "    <input type=\"submit\" value=\"Submit\"/>\r\n"
           "</form>\r\n"
           "</body>\r\n"
           "</html>\r\n";
}

std::string buildListingHtml(const std::string& caption,
                             const std::string& date,
                             const std::string& savedName,
                             const std::vector<std::string>& files)
{
    std::ostringstream oss;
    oss << "<!DOCTYPE html>\r\n"
        << "<html lang=\"en\">\r\n"
        << "<head><meta charset=\"UTF-8\"/><title>Upload Result</title></head>\r\n"
        << "<body>\r\n"
        << "<h2>Upload Successful</h2>\r\n"
        << "<p>Caption: " << escapeHtml(caption) << "</p>\r\n"
        << "<p>Date: " << escapeHtml(date) << "</p>\r\n"
        << "<p>Saved as: " << escapeHtml(savedName) << "</p>\r\n"
        << "<h3>images/ Directory (alphabetical)</h3>\r\n"
        << "<ul>\r\n";
    if(files.empty())
    {
        oss << "    <li><em>No files uploaded yet</em></li>\r\n";
    }
    else
    {
        for(const auto& file : files)
        {
            oss << "    <li>" << escapeHtml(file) << "</li>\r\n";
        }
    }
    oss << "</ul>\r\n"
        << "<hr><a href='/'>&larr; Back to form</a>\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    return oss.str();
}

void parseContentDisposition(const std::string& disposition, FormPart& part)
{
    std::size_t pos = 0;
    while(pos < disposition.size())
    {
        while(pos < disposition.size() && std::isspace(static_cast<unsigned char>(disposition[pos])))
        {
            ++pos;
        }
        std::size_t next = disposition.find(';', pos);
        std::string token = disposition.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
        std::size_t eq = token.find('=');
        if(eq != std::string::npos)
        {
            std::string name = token.substr(0, eq);
            std::string value = token.substr(eq + 1);
            if(!value.empty() && value.front() == '"' && value.back() == '"' && value.size() >= 2)
            {
                value = value.substr(1, value.size() - 2);
            }
            if(name == "name")
            {
                part.name = value;
            }
            else if(name == "filename")
            {
                part.filename = value;
            }
        }
        if(next == std::string::npos)
        {
            break;
        }
        pos = next + 1;
    }
}

std::map<std::string, std::string> parseHeaders(const std::string& headerBlock)
{
    std::map<std::string, std::string> headers;
    std::size_t start = 0;
    while(start < headerBlock.size())
    {
        std::size_t end = headerBlock.find("\r\n", start);
        if(end == std::string::npos)
        {
            end = headerBlock.size();
        }
        std::string line = headerBlock.substr(start, end - start);
        if(line.empty())
        {
            break;
        }
        std::size_t colon = line.find(':');
        if(colon != std::string::npos)
        {
            std::string name = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while(!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
            {
                value.erase(value.begin());
            }
            while(!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
            {
                value.pop_back();
            }
            headers[name] = value;
        }
        start = end + 2;
    }
    return headers;
}

std::map<std::string, FormPart> parseMultipart(const std::vector<std::uint8_t>& body,
                                               const std::string& boundary)
{
    std::map<std::string, FormPart> parts;
    const std::string delimiter = "--" + boundary;
    const std::string closeDelimiter = delimiter + "--";

    std::string bodyStr(reinterpret_cast<const char*>(body.data()), body.size());
    std::size_t pos = 0;
    while(true)
    {
        std::size_t boundaryPos = bodyStr.find(delimiter, pos);
        if(boundaryPos == std::string::npos)
        {
            break;
        }
        boundaryPos += delimiter.size();
        if(boundaryPos >= bodyStr.size())
        {
            break;
        }
        if(bodyStr.compare(boundaryPos, 2, "--") == 0)
        {
            break;
        }
        if(bodyStr.compare(boundaryPos, 2, "\r\n") == 0)
        {
            boundaryPos += 2;
        }
        else if(bodyStr.compare(boundaryPos, 1, "\n") == 0)
        {
            boundaryPos += 1;
        }

        std::size_t nextBoundary = bodyStr.find(delimiter, boundaryPos);
        std::size_t closingBoundary = bodyStr.find(closeDelimiter, boundaryPos);
        if(nextBoundary == std::string::npos || (closingBoundary != std::string::npos && closingBoundary < nextBoundary))
        {
            nextBoundary = closingBoundary;
        }
        if(nextBoundary == std::string::npos)
        {
            nextBoundary = bodyStr.size();
        }
        std::size_t partEnd = nextBoundary;
        while(partEnd > boundaryPos && (bodyStr[partEnd - 1] == '\r' || bodyStr[partEnd - 1] == '\n'))
        {
            --partEnd;
        }

        std::string partStr = bodyStr.substr(boundaryPos, partEnd - boundaryPos);
        std::size_t headerEnd = partStr.find("\r\n\r\n");
        std::size_t sepLen = 4;
        if(headerEnd == std::string::npos)
        {
            headerEnd = partStr.find("\n\n");
            sepLen = 2;
        }
        if(headerEnd == std::string::npos)
        {
            throw std::runtime_error("Malformed multipart segment");
        }
        std::string headerBlock = partStr.substr(0, headerEnd);
        std::vector<std::uint8_t> data(partStr.begin() + headerEnd + sepLen, partStr.end());

        FormPart part;
        part.headers = parseHeaders(headerBlock);
        auto dispIt = part.headers.find("Content-Disposition");
        if(dispIt == part.headers.end())
        {
            throw std::runtime_error("Missing Content-Disposition header");
        }
        parseContentDisposition(dispIt->second, part);
        part.data = std::move(data);
        if(!part.name.empty())
        {
            parts[part.name] = std::move(part);
        }

        pos = nextBoundary;
        if(nextBoundary == closingBoundary)
        {
            break;
        }
    }
    return parts;
}
}

void UploadServlet::doGet(const HttpServletRequest& request, HttpServletResponse& response)
{
    (void)request;
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.outputStream() << loadFormHtml();
}

void UploadServlet::doPost(const HttpServletRequest& request, HttpServletResponse& response)
{
    try
    {
        std::string contentType = request.header("content-type");
        if(contentType.empty() || contentType.find("multipart/form-data") == std::string::npos)
        {
            throw std::runtime_error("Missing multipart/form-data content type");
        }

        std::size_t boundaryPos = contentType.find("boundary=");
        if(boundaryPos == std::string::npos)
        {
            throw std::runtime_error("Missing multipart boundary");
        }
        std::string boundary = contentType.substr(boundaryPos + 9);
        if(!boundary.empty() && boundary.front() == '"' && boundary.back() == '"' && boundary.size() >= 2)
        {
            boundary = boundary.substr(1, boundary.size() - 2);
        }

        auto parts = parseMultipart(request.body(), boundary);
        auto fileIt = parts.find("fileName");
        if(fileIt == parts.end() || fileIt->second.filename.empty() || fileIt->second.data.empty())
        {
            throw std::runtime_error("Missing uploaded file");
        }

        std::string caption = parts.count("caption") ? parts["caption"].asText() : std::string();
        std::string date = parts.count("date") ? parts["date"].asText() : std::string();

        auto& filePart = fileIt->second;
        auto path = UploadDirectoryManager::instance().createTargetFile(caption, date, filePart.filename);
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(filePart.data.data()), static_cast<std::streamsize>(filePart.data.size()));
        out.close();

        auto files = UploadDirectoryManager::instance().listFilesSorted();
        response.setHeader("Content-Type", "text/html; charset=UTF-8");
        response.outputStream() << buildListingHtml(caption, date, path.filename().string(), files);
    }
    catch(const std::exception& ex)
    {
        response.setStatus(400, "Bad Request");
        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
        response.outputStream() << "Bad Request: " << ex.what();
    }
}
