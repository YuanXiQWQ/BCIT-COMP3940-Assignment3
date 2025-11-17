#pragma once
#include "HttpServletRequest.hpp"
#include "Socket.h"
#include "Thread.h"

class UploadServerThread : public Thread {
    Socket* socket;

public:
    UploadServerThread(Socket* socket);

    void run();

    std::stringstream& parseRequest(std::stringstream& is, HttpServletRequest& req);
};
