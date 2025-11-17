#include "UploadServerThread.h"

#include "HttpServletRequest.h"
#include "HttpServletResponse.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cctype>

namespace {
std::string trim(const std::string& value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}
}

UploadServerThread::UploadServerThread(std::unique_ptr<Socket> clientSocket, HttpServlet& servlet)
    : socket(std::move(clientSocket)), servlet(servlet)
{
}

void UploadServerThread::run()
{
    try {
        processRequest();
    } catch (...) {
        if (socket && socket->isOpen()) {
            try {
                socket->close();
            } catch (...) {
            }
        }
    }
}

void UploadServerThread::processRequest()
{
    if (!socket || !socket->isOpen()) {
        return;
    }

    std::string requestLine = socket->readLine();
    if (requestLine.empty()) {
        return;
    }

    std::istringstream rl(requestLine);
    std::string method;
    std::string target;
    std::string version;
    rl >> method >> target >> version;
    if (method.empty() || target.empty()) {
        return;
    }
    if (version.empty()) {
        version = "HTTP/1.1";
    }

    std::string headersBlock = socket->readHeadersBlock();
    auto headers = parseHeaders(headersBlock);

    std::size_t contentLength = 0;
    auto clIt = headers.find("content-length");
    if (clIt != headers.end()) {
        contentLength = static_cast<std::size_t>(std::stoul(clIt->second));
    }

    std::vector<char> body;
    if (contentLength > 0) {
        body = socket->readBytes(contentLength);
    }

    std::string path = target;
    std::string query;
    auto qpos = target.find('?');
    if (qpos != std::string::npos) {
        path = target.substr(0, qpos);
        query = target.substr(qpos + 1);
    }

    HttpServletRequest request(method, path, version, headers, body);
    if (!query.empty()) {
        parseQueryString(query, request);
    }

    HttpServletResponse response;
    servlet.service(request, response);
    auto payload = response.buildResponse();
    socket->sendAll(payload.data(), payload.size());
    socket->shutdownWrite();
    socket->close();
}

std::unordered_map<std::string, std::string> UploadServerThread::parseHeaders(const std::string& headersBlock)
{
    std::unordered_map<std::string, std::string> headers;
    for (const auto& line : splitHeaderLines(headersBlock)) {
        auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        headers.emplace(normalizeHeaderName(trim(name)), trim(value));
    }
    return headers;
}

std::vector<std::string> UploadServerThread::splitHeaderLines(const std::string& headersBlock)
{
    std::vector<std::string> lines;
    std::istringstream stream(headersBlock);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

std::string UploadServerThread::normalizeHeaderName(const std::string& name)
{
    std::string lowered = name;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

void UploadServerThread::parseQueryString(const std::string& query, HttpServletRequest& request)
{
    std::size_t start = 0;
    while (start < query.size()) {
        std::size_t eq = query.find('=', start);
        if (eq == std::string::npos) {
            break;
        }
        std::size_t end = query.find('&', eq);
        std::string key = query.substr(start, eq - start);
        std::string value = end == std::string::npos ? query.substr(eq + 1)
                                                     : query.substr(eq + 1, end - (eq + 1));
        request.setParameter(key, value);
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
}
