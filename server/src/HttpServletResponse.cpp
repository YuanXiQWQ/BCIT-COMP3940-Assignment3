#include "HttpServletResponse.hpp"

#include <iomanip>

HttpServletResponse::HttpServletResponse() : statusCode_(200), statusMessage_("OK")
{
    setHeader("Content-Type", "text/html; charset=UTF-8");
}

void HttpServletResponse::setStatus(int code, std::string message)
{
    statusCode_ = code;
    statusMessage_ = std::move(message);
}

void HttpServletResponse::setHeader(const std::string& name, const std::string& value)
{
    headers_[name] = value;
}

std::ostream& HttpServletResponse::outputStream()
{
    return body_;
}

std::string HttpServletResponse::buildResponse() const
{
    std::ostringstream oss;
    std::string bodyStr = body_.str();

    oss << "HTTP/1.1 " << statusCode_ << ' ' << statusMessage_ << "\r\n";
    for(const auto& [name, value] : headers_)
    {
        oss << name << ": " << value << "\r\n";
    }
    oss << "Content-Length: " << bodyStr.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << bodyStr;
    return oss.str();
}
