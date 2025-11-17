#include "Url.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

Url::Url(std::string rawUrl)
    : raw(std::move(rawUrl)), schemeValue(), hostValue(), portValue(0), pathValue()
{
    parse();
}

const std::string& Url::scheme() const
{
    return schemeValue;
}

const std::string& Url::host() const
{
    return hostValue;
}

int Url::port() const
{
    return portValue;
}

const std::string& Url::path() const
{
    return pathValue;
}

void Url::parse()
{
    const std::string schemeMarker = "://";
    auto schemeEnd = raw.find(schemeMarker);
    if (schemeEnd == std::string::npos) {
        throw std::invalid_argument("URL must include scheme://");
    }

    schemeValue = raw.substr(0, schemeEnd);
    std::transform(schemeValue.begin(), schemeValue.end(), schemeValue.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    std::string remainder = raw.substr(schemeEnd + schemeMarker.size());
    if (remainder.empty()) {
        throw std::invalid_argument("URL host is missing");
    }

    auto slashPos = remainder.find('/');
    if (slashPos == std::string::npos) {
        hostValue = remainder;
        pathValue = "/";
    } else {
        hostValue = remainder.substr(0, slashPos);
        pathValue = remainder.substr(slashPos);
        if (pathValue.empty()) {
            pathValue = "/";
        }
    }

    auto colonPos = hostValue.find(':');
    if (colonPos != std::string::npos) {
        std::string portStr = hostValue.substr(colonPos + 1);
        hostValue = hostValue.substr(0, colonPos);
        if (portStr.empty()) {
            throw std::invalid_argument("Port is empty in URL");
        }
        portValue = std::stoi(portStr);
    } else {
        portValue = schemeValue == "https" ? 443 : 80;
    }

    if (hostValue.empty()) {
        throw std::invalid_argument("Host is required in URL");
    }
}
