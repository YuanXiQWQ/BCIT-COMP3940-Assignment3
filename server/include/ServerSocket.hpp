#pragma once

#include "Socket.hpp"

#include <netinet/in.h>

#include <cstdint>
#include <string>

class ServerSocket {
public:
    ServerSocket();
    explicit ServerSocket(std::uint16_t port, int backlog = 16);
    ServerSocket(const ServerSocket&) = delete;
    ServerSocket& operator=(const ServerSocket&) = delete;
    ServerSocket(ServerSocket&& other) noexcept;
    ServerSocket& operator=(ServerSocket&& other) noexcept;
    ~ServerSocket();

    void bindAndListen(std::uint16_t port, int backlog = 16);
    Socket accept();
    void close();

private:
    int fd_;
    sockaddr_in addr_{};
};
