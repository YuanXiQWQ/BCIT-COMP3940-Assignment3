#include "Socket.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

Socket::Socket(int sockFd) : sock(sockFd) {}

Socket::~Socket()
{
    if (isOpen()) {
        close();
    }
}

Socket::Socket(Socket&& other) noexcept : sock(other.sock)
{
    other.sock = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other) {
        if (isOpen()) {
            close();
        }
        sock = other.sock;
        other.sock = -1;
    }
    return *this;
}

std::string Socket::readLine()
{
    std::string line;
    char ch;
    while (true) {
        int n = rawRecv(&ch, 1);
        if (n <= 0) {
            break;
        }
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            break;
        }
        line.push_back(ch);
    }
    return line;
}

std::string Socket::readHeadersBlock()
{
    std::string headers;
    std::string line;
    while (!(line = readLine()).empty()) {
        headers.append(line).append("\r\n");
    }
    return headers;
}

std::vector<char> Socket::readBytes(std::size_t length)
{
    std::vector<char> buffer(length);
    std::size_t offset = 0;
    while (offset < length) {
        int n = rawRecv(buffer.data() + offset, length - offset);
        if (n <= 0) {
            throw std::runtime_error("Socket closed while reading body");
        }
        offset += static_cast<std::size_t>(n);
    }
    return buffer;
}

std::vector<char> Socket::readRemaining()
{
    std::vector<char> data;
    char buffer[4096];
    int n;
    while ((n = rawRecv(buffer, sizeof(buffer))) > 0) {
        data.insert(data.end(), buffer, buffer + n);
        if (n < static_cast<int>(sizeof(buffer))) {
            break;
        }
    }
    return data;
}

int Socket::receive(char* buffer, std::size_t length)
{
    return rawRecv(buffer, length);
}

void Socket::sendAll(const std::string& data)
{
    sendAll(data.data(), data.size());
}

void Socket::sendAll(const char* data, std::size_t length)
{
    std::size_t offset = 0;
    while (offset < length) {
        int n = rawSend(data + offset, length - offset);
        if (n <= 0) {
            throw std::runtime_error("Socket send failure");
        }
        offset += static_cast<std::size_t>(n);
    }
}

void Socket::shutdownWrite()
{
    if (!isOpen()) {
        return;
    }
#ifdef _WIN32
    ::shutdown(sock, SD_SEND);
#else
    ::shutdown(sock, SHUT_WR);
#endif
}

void Socket::close()
{
    if (!isOpen()) {
        return;
    }
#ifdef _WIN32
    closesocket(sock);
#else
    ::close(sock);
#endif
    sock = -1;
}

bool Socket::isOpen() const
{
    return sock >= 0;
}

int Socket::rawRecv(char* buffer, std::size_t length)
{
#ifdef _WIN32
    return ::recv(sock, buffer, static_cast<int>(length), 0);
#else
    return static_cast<int>(::recv(sock, buffer, length, 0));
#endif
}

int Socket::rawSend(const char* buffer, std::size_t length)
{
#ifdef _WIN32
    return ::send(sock, buffer, static_cast<int>(length), 0);
#else
    return static_cast<int>(::send(sock, buffer, length, 0));
#endif
}
