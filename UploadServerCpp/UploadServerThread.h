#pragma once

#include "Thread.h"
#include "Socket.h"
#include "HttpServlet.h"

#include <memory>
#include <string>
#include <unordered_map>

class UploadServerThread : public Thread {
public:
    UploadServerThread(std::unique_ptr<Socket> clientSocket, HttpServlet& servlet);
    ~UploadServerThread() override = default;

protected:
    void run() override;

private:
    std::unique_ptr<Socket> socket;
    HttpServlet& servlet;

    void processRequest();
    static std::unordered_map<std::string, std::string> parseHeaders(const std::string& headersBlock);
    static std::vector<std::string> splitHeaderLines(const std::string& headersBlock);
    static std::string normalizeHeaderName(const std::string& name);
    static void parseQueryString(const std::string& query, HttpServletRequest& request);
};
