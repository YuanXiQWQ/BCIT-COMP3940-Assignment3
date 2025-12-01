#include "HttpServletRequest.hpp"

#include <algorithm>
#include <cctype>

namespace {
std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}
}

HttpServletRequest::HttpServletRequest(std::string method,
                                       std::string uri,
                                       std::map<std::string, std::string> headers,
                                       std::vector<std::uint8_t> body)
        : method_(std::move(method)),
          uri_(std::move(uri)),
          headers_(std::move(headers)),
          body_(std::move(body))
{
    for(auto it = headers_.begin(); it != headers_.end();)
    {
        auto node = headers_.extract(it++);
        std::string keyLower = toLower(node.key());
        node.key() = keyLower;
        headers_.insert(std::move(node));
    }
}

const std::string& HttpServletRequest::method() const
{
    return method_;
}

const std::string& HttpServletRequest::uri() const
{
    return uri_;
}

const std::map<std::string, std::string>& HttpServletRequest::headers() const
{
    return headers_;
}

std::string HttpServletRequest::header(const std::string& name) const
{
    auto it = headers_.find(toLower(name));
    return it == headers_.end() ? std::string() : it->second;
}

const std::vector<std::uint8_t>& HttpServletRequest::body() const
{
    return body_;
}
