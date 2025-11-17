#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct UploadedFile{
    std::string fieldName;
    std::string originalFileName;
    std::string contentType;
    std::vector<char> data;
};

class HttpServletRequest{
public:
    HttpServletRequest(std::string method,
                       std::string path,
                       std::string version,
                       std::unordered_map<std::string, std::string> headers,
                       std::vector<char> body);

    const std::string &getMethod() const;

    const std::string &getPath() const;

    const std::string &getHttpVersion() const;

    bool hasHeader(const std::string &name) const;

    std::optional<std::string> getHeader(const std::string &name) const;

    void setParameter(const std::string &name, const std::string &value);

    std::optional<std::string> getParameter(const std::string &name) const;

    std::string getCaption() const;

    std::string getDate() const;

    void setUploadedFile(UploadedFile file);

    bool hasUploadedFile() const;

    const UploadedFile &getUploadedFile() const;

    const std::vector<char> &getBody() const;

private:
    static std::string normalizeHeaderName(const std::string &raw);

    std::string method;
    std::string path;
    std::string httpVersion;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> parameters;
    std::vector<char> body;
    std::optional<UploadedFile> uploadedFile;
};
