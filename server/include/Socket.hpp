#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Socket {
public:
    explicit Socket(int fd = -1);
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] int descriptor() const;

    std::size_t readSome(std::vector<std::uint8_t>& buffer, std::size_t offset);
    std::size_t readExact(std::vector<std::uint8_t>& buffer, std::size_t bytesNeeded);
    std::size_t writeAll(const std::uint8_t* data, std::size_t size);
    std::size_t writeAll(const std::string& data);

    void shutdownWrite();
    void close();

private:
    int fd_;
};
