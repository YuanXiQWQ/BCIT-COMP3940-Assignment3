#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Socket {
public:
    explicit Socket(int sockFd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    std::string readLine();
    std::string readHeadersBlock();
    std::vector<char> readBytes(std::size_t length);
    std::vector<char> readRemaining();
    int receive(char* buffer, std::size_t length);
    void sendAll(const std::string& data);
    void sendAll(const char* data, std::size_t length);
    void shutdownWrite();
    void close();

    bool isOpen() const;

private:
    int sock;

    int rawRecv(char* buffer, std::size_t length);
    int rawSend(const char* buffer, std::size_t length);
};
