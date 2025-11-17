#include "HttpClient.h"

#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

HttpClient::HttpClient() : socketFd(-1)
{
#ifdef _WIN32
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

HttpClient::~HttpClient()
{
    closeSocket();
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpClient::ensureSocket()
{
    if (socketFd >= 0) {
        return;
    }
    socketFd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (socketFd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

void HttpClient::closeSocket()
{
    if (socketFd >= 0) {
#ifdef _WIN32
        closesocket(socketFd);
#else
        ::close(socketFd);
#endif
        socketFd = -1;
    }
}

void HttpClient::connectSocket(const Url& url)
{
    ensureSocket();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    int rc = getaddrinfo(url.host().c_str(), std::to_string(url.port()).c_str(), &hints, &result);
    if (rc != 0) {
#ifdef _WIN32
        closeSocket();
        throw std::runtime_error("getaddrinfo failed");
#else
        closeSocket();
        throw std::runtime_error(gai_strerror(rc));
#endif
    }

    bool connected = false;
    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (::connect(socketFd, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == 0) {
            connected = true;
            break;
        }
    }
    freeaddrinfo(result);

    if (!connected) {
        closeSocket();
        throw std::runtime_error("Unable to connect to host");
    }
}

void HttpClient::sendAll(const char* data, std::size_t length)
{
    std::size_t offset = 0;
    while (offset < length) {
#ifdef _WIN32
        int sent = ::send(socketFd, data + offset, static_cast<int>(length - offset), 0);
#else
        int sent = static_cast<int>(::send(socketFd, data + offset, length - offset, 0));
#endif
        if (sent <= 0) {
            throw std::runtime_error("Socket send failed");
        }
        offset += static_cast<std::size_t>(sent);
    }
}

std::string HttpClient::readResponse()
{
    std::string response;
    char buffer[4096];
    int received;
    while (true) {
#ifdef _WIN32
        received = ::recv(socketFd, buffer, sizeof(buffer), 0);
#else
        received = static_cast<int>(::recv(socketFd, buffer, sizeof(buffer), 0));
#endif
        if (received <= 0) {
            break;
        }
        response.append(buffer, buffer + received);
        if (received < static_cast<int>(sizeof(buffer))) {
            break;
        }
    }
    return response;
}

std::string HttpClient::get(const Url& url, const std::vector<std::string>& headers)
{
    connectSocket(url);

    std::string request = "GET " + url.path() + " HTTP/1.1\r\n";
    request += "Host: " + url.host() + "\r\n";
    for (const auto& header : headers) {
        request += header + "\r\n";
    }
    request += "Connection: close\r\n\r\n";

    sendAll(request.data(), request.size());
    std::string response = readResponse();
    closeSocket();
    return response;
}

std::string HttpClient::post(const Url& url,
                             const std::vector<std::string>& headers,
                             const std::vector<char>& body)
{
    connectSocket(url);

    std::string request = "POST " + url.path() + " HTTP/1.1\r\n";
    request += "Host: " + url.host() + "\r\n";
    request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    for (const auto& header : headers) {
        request += header + "\r\n";
    }
    request += "Connection: close\r\n\r\n";

    sendAll(request.data(), request.size());
    if (!body.empty()) {
        sendAll(body.data(), body.size());
    }

    std::string response = readResponse();
    closeSocket();
    return response;
}
