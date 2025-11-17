#pragma once

#include <string>

class Url {
public:
    explicit Url(std::string rawUrl);

    const std::string& scheme() const;
    const std::string& host() const;
    int port() const;
    const std::string& path() const;

private:
    std::string raw;
    std::string schemeValue;
    std::string hostValue;
    int portValue;
    std::string pathValue;

    void parse();
};
