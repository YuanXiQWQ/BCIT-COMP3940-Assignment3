#include "UploadServerThread.hpp"

#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr std::size_t kHeaderChunk = 4096;

std::string toLower(const std::string& value)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lower;
}

std::string trim(const std::string& value)
{
    std::size_t start = 0;
    while(start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
    {
        ++start;
    }
    std::size_t end = value.size();
    while(end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
    {
        --end;
    }
    return value.substr(start, end - start);
}
}

UploadServerThread::UploadServerThread(Socket socket) : socket_(std::move(socket)) {}

void UploadServerThread::operator()()
{
    try
    {
        handleConnection();
    }
    catch(const std::exception& ex)
    {
        try
        {
            HttpServletResponse response;
            response.setStatus(500, "Internal Server Error");
            response.setHeader("Content-Type", "text/plain; charset=UTF-8");
            response.outputStream() << "Internal Server Error: " << ex.what();
            std::string responseStr = response.buildResponse();
            socket_.writeAll(responseStr);
        }
        catch(...)
        {
        }
    }
    socket_.close();
}

void UploadServerThread::handleConnection()
{
    std::vector<std::uint8_t> buffer;
    buffer.reserve(kHeaderChunk);

    std::size_t total = 0;
    std::size_t headerEnd = std::string::npos;
    std::string headerStr;

    while(headerEnd == std::string::npos)
    {
        std::size_t before = buffer.size();
        std::size_t n = socket_.readSome(buffer, total);
        if(n == 0)
        {
            break;
        }
        total += n;
        headerStr.assign(reinterpret_cast<const char*>(buffer.data()), total);
        headerEnd = headerStr.find("\r\n\r\n");
        if(headerEnd == std::string::npos)
        {
            headerEnd = headerStr.find("\n\n");
        }
    }

    if(headerEnd == std::string::npos)
    {
        throw std::runtime_error("Malformed HTTP request");
    }

    std::size_t bodyOffset = headerEnd + (headerStr.compare(headerEnd, 4, "\r\n\r\n") == 0 ? 4 : 2);
    if(total < bodyOffset)
    {
        bodyOffset = total;
    }

    std::string headerSection(headerStr.data(), headerEnd);
    std::istringstream headerStream(headerSection);
    std::string requestLine;
    std::getline(headerStream, requestLine);
    requestLine = trim(requestLine);
    if(requestLine.empty())
    {
        throw std::runtime_error("Missing request line");
    }

    std::istringstream requestLineStream(requestLine);
    std::string method;
    std::string uri;
    std::string version;
    requestLineStream >> method >> uri >> version;
    if(method.empty())
    {
        throw std::runtime_error("Missing HTTP method");
    }
    if(uri.empty())
    {
        uri = "/";
    }

    std::map<std::string, std::string> headers;
    std::string line;
    while(std::getline(headerStream, line))
    {
        if(line.size() >= 1 && line.back() == '\r')
        {
            line.pop_back();
        }
        if(line.empty())
        {
            break;
        }
        std::size_t colon = line.find(':');
        if(colon != std::string::npos)
        {
            std::string name = trim(line.substr(0, colon));
            std::string value = trim(line.substr(colon + 1));
            headers[toLower(name)] = value;
        }
    }

    std::size_t contentLength = 0;
    if(headers.count("content-length"))
    {
        contentLength = static_cast<std::size_t>(std::stoul(headers["content-length"]));
    }

    std::vector<std::uint8_t> body;
    if(contentLength > 0)
    {
        body.assign(buffer.begin() + bodyOffset, buffer.end());
        std::size_t remaining = contentLength > body.size() ? contentLength - body.size() : 0;
        if(remaining > 0)
        {
            std::vector<std::uint8_t> extra;
            extra.reserve(remaining);
            socket_.readExact(extra, remaining);
            body.insert(body.end(), extra.begin(), extra.end());
        }
    }

    HttpServletRequest request(method, uri, headers, std::move(body));
    HttpServletResponse response;

    std::string methodUpper = toLower(method);
    if(methodUpper == "get")
    {
        servlet_.doGet(request, response);
    }
    else if(methodUpper == "post")
    {
        servlet_.doPost(request, response);
    }
    else
    {
        response.setStatus(405, "Method Not Allowed");
        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
        response.outputStream() << "Method Not Allowed";
    }

    std::string responseStr = response.buildResponse();
    socket_.writeAll(responseStr);
}
