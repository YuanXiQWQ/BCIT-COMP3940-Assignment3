#include "Socket.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace {
constexpr std::size_t kDefaultBufferChunk = 8192;

void throwSystemError(const char* action)
{
    throw std::runtime_error(std::string(action) + ": " + std::strerror(errno));
}
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_)
{
    other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if(this != &other)
    {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

Socket::~Socket()
{
    close();
}

bool Socket::isValid() const
{
    return fd_ >= 0;
}

int Socket::descriptor() const
{
    return fd_;
}

std::size_t Socket::readSome(std::vector<std::uint8_t>& buffer, std::size_t offset)
{
    if(!isValid())
    {
        throw std::runtime_error("Attempt to read from invalid socket");
    }
    if(buffer.size() < offset + kDefaultBufferChunk)
    {
        buffer.resize(offset + kDefaultBufferChunk);
    }
    ssize_t n = ::read(fd_, buffer.data() + offset, kDefaultBufferChunk);
    if(n < 0)
    {
        if(errno == EINTR)
        {
            return 0;
        }
        throwSystemError("read");
    }
    return static_cast<std::size_t>(n);
}

std::size_t Socket::readExact(std::vector<std::uint8_t>& buffer, std::size_t bytesNeeded)
{
    std::size_t offset = buffer.size();
    buffer.resize(offset + bytesNeeded);
    std::size_t total = 0;
    while(total < bytesNeeded)
    {
        ssize_t n = ::read(fd_, buffer.data() + offset + total, bytesNeeded - total);
        if(n < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            throwSystemError("read");
        }
        if(n == 0)
        {
            break;
        }
        total += static_cast<std::size_t>(n);
    }
    buffer.resize(offset + total);
    return total;
}

std::size_t Socket::writeAll(const std::uint8_t* data, std::size_t size)
{
    if(!isValid())
    {
        throw std::runtime_error("Attempt to write to invalid socket");
    }
    std::size_t written = 0;
    while(written < size)
    {
        ssize_t n = ::write(fd_, data + written, size - written);
        if(n < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            throwSystemError("write");
        }
        written += static_cast<std::size_t>(n);
    }
    return written;
}

std::size_t Socket::writeAll(const std::string& data)
{
    return writeAll(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

void Socket::shutdownWrite()
{
    if(isValid())
    {
        ::shutdown(fd_, SHUT_WR);
    }
}

void Socket::close()
{
    if(isValid())
    {
        ::close(fd_);
        fd_ = -1;
    }
}
