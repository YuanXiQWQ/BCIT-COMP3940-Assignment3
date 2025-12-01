#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class HttpServletRequest {
public:
    HttpServletRequest(std::string method,
                       std::string uri,
                       std::map<std::string, std::string> headers,
                       std::vector<std::uint8_t> body);

    [[nodiscard]] const std::string& method() const;
    [[nodiscard]] const std::string& uri() const;
    [[nodiscard]] const std::map<std::string, std::string>& headers() const;
    [[nodiscard]] std::string header(const std::string& name) const;
    [[nodiscard]] const std::vector<std::uint8_t>& body() const;

private:
    std::string method_;
    std::string uri_;
    std::map<std::string, std::string> headers_;
    std::vector<std::uint8_t> body_;
};
