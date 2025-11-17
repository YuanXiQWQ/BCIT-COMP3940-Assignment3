#pragma once

#include "HttpServletRequest.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class MultipartParser{
public:
    struct Part{
        std::unordered_map<std::string, std::string> headers;
        std::vector<char> data;
    };

    static bool parse(HttpServletRequest &request);

private:
    static std::string extractBoundary(const std::string &contentType);

    static std::vector<Part>
    splitParts(const std::vector<char> &body, const std::string &boundary);

    static std::unordered_map<std::string, std::string>
    parsePartHeaders(const std::string &headerBlock);

    static std::string normalizeHeaderName(const std::string &name);

    static std::unordered_map<std::string, std::string>
    parseContentDisposition(const std::string &headerValue);
};
