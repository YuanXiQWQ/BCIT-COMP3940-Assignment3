#pragma once

#include "HttpServlet.hpp"
#include "Socket.hpp"
#include "UploadServlet.hpp"

#include <string>

class UploadServerThread {
public:
    explicit UploadServerThread(Socket socket);
    void operator()();

private:
    Socket socket_;
    UploadServlet servlet_;

    void handleConnection();
};
