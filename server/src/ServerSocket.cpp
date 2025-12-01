#include "ServerSocket.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace {
void throwSystemError(const char* action)
{
    throw std::runtime_error(std::string(action) + ": " + std::strerror(errno));
}
}

ServerSocket::ServerSocket() : fd_(-1) {}

ServerSocket::ServerSocket(std::uint16_t port, int backlog) : fd_(-1)
{
    bindAndListen(port, backlog);
}

ServerSocket::ServerSocket(ServerSocket&& other) noexcept : fd_(other.fd_), addr_(other.addr_)
{
    other.fd_ = -1;
}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) noexcept
{
    if(this != &other)
    {
        close();
        fd_ = other.fd_;
        addr_ = other.addr_;
        other.fd_ = -1;
    }
    return *this;
}

ServerSocket::~ServerSocket()
{
    close();
}

void ServerSocket::bindAndListen(std::uint16_t port, int backlog)
{
    close();

    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if(fd_ < 0)
    {
        throwSystemError("socket");
    }

    int opt = 1;
    if(::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throwSystemError("setsockopt");
    }

    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);

    if(::bind(fd_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) < 0)
    {
        throwSystemError("bind");
    }

    if(::listen(fd_, backlog) < 0)
    {
        throwSystemError("listen");
    }
}

Socket ServerSocket::accept()
{
    if(fd_ < 0)
    {
        throw std::runtime_error("accept on closed server socket");
    }
    int clientFd = ::accept(fd_, nullptr, nullptr);
    if(clientFd < 0)
    {
        throwSystemError("accept");
    }
    return Socket(clientFd);
}

void ServerSocket::close()
{
    if(fd_ >= 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
}
