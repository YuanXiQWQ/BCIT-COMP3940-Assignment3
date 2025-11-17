#pragma once

#include "Socket.h"

#include <memory>

class ServerSocket{
public:
    explicit ServerSocket(int port);

    std::unique_ptr<Socket> Accept();

    ~ServerSocket();

private:
    int sock;
};
