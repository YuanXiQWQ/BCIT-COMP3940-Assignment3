#pragma once

#include "Url.h"

#include <string>
#include <vector>

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    std::string get(const Url& url, const std::vector<std::string>& headers = {});
    std::string post(const Url& url,
                     const std::vector<std::string>& headers,
                     const std::vector<char>& body);

private:
    int socketFd;

    void ensureSocket();
    void closeSocket();
    void connectSocket(const Url& url);
    std::string readResponse();
    void sendAll(const char* data, std::size_t length);
};
