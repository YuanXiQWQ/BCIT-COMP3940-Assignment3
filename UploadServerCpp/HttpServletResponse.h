#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class HttpServletResponse{
public:
    HttpServletResponse();

    void setStatus(int code, std::string message);

    void setHeader(const std::string &name, const std::string &value);

    void setContentType(const std::string &value);

    void write(const std::string &text);

    void writeBinary(const std::vector<char> &data);

    void writeBinary(const char *data, std::size_t length);

    std::vector<char> buildResponse() const;

private:
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::vector<char> body;
};
