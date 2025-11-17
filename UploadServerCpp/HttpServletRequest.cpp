#include "HttpServletRequest.h"

#include <algorithm>
#include <stdexcept>
#include <cctype>

namespace{
    std::string toLower(const std::string &value){
        std::string lowered = value;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                       [](unsigned char ch){
                           return static_cast<char>(std::tolower(ch));
                       });
        return lowered;
    }
}

HttpServletRequest::HttpServletRequest(std::string method,
                                       std::string path,
                                       std::string version,
                                       std::unordered_map<std::string, std::string> headers,
                                       std::vector<char> body)
        : method(std::move(method)),
          path(std::move(path)),
          httpVersion(std::move(version)),
          headers(),
          parameters(),
          body(std::move(body)),
          uploadedFile(std::nullopt){
    for(auto &entry: headers){
        this->headers.emplace(normalizeHeaderName(entry.first), entry.second);
    }
}

const std::string &HttpServletRequest::getMethod() const{
    return method;
}

const std::string &HttpServletRequest::getPath() const{
    return path;
}

const std::string &HttpServletRequest::getHttpVersion() const{
    return httpVersion;
}

bool HttpServletRequest::hasHeader(const std::string &name) const{
    return headers.find(normalizeHeaderName(name)) != headers.end();
}

std::optional<std::string> HttpServletRequest::getHeader(const std::string &name) const{
    auto it = headers.find(normalizeHeaderName(name));
    if(it == headers.end()){
        return std::nullopt;
    }
    return it->second;
}

void HttpServletRequest::setParameter(const std::string &name, const std::string &value){
    parameters[name] = value;
}

std::optional<std::string>
HttpServletRequest::getParameter(const std::string &name) const{
    auto it = parameters.find(name);
    if(it == parameters.end()){
        return std::nullopt;
    }
    return it->second;
}

std::string HttpServletRequest::getCaption() const{
    auto value = getParameter("caption");
    return value.value_or("");
}

std::string HttpServletRequest::getDate() const{
    auto value = getParameter("date");
    return value.value_or("");
}

void HttpServletRequest::setUploadedFile(UploadedFile file){
    uploadedFile = std::move(file);
}

bool HttpServletRequest::hasUploadedFile() const{
    return uploadedFile.has_value();
}

const UploadedFile &HttpServletRequest::getUploadedFile() const{
    if(! uploadedFile.has_value()){
        throw std::logic_error("No uploaded file present");
    }
    return uploadedFile.value();
}

const std::vector<char> &HttpServletRequest::getBody() const{
    return body;
}

std::string HttpServletRequest::normalizeHeaderName(const std::string &raw){
    return toLower(raw);
}
