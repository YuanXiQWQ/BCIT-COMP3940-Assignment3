#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

class HttpServletResponse {
public:
    HttpServletResponse();

    void setStatus(int code, std::string message);
    void setHeader(const std::string& name, const std::string& value);
    std::ostream& outputStream();

    std::string buildResponse() const;

private:
    int statusCode_;
    std::string statusMessage_;
    std::map<std::string, std::string> headers_;
    std::ostringstream body_;
};
