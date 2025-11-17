#pragma once

class Socket{
public:
    Socket(int sock);

    char *getRequest();

    void sendResponse(char *res);

    ~Socket();

private:
    int sock;
};

