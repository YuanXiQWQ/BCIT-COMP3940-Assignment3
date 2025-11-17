#include "MultipartForm.h"

#include <sstream>

MultipartForm::MultipartForm(std::string boundary)
    : boundaryValue(std::move(boundary)), parts()
{
}

void MultipartForm::addField(const std::string& name, const std::string& value)
{
    std::ostringstream part;
    part << "Content-Disposition: form-data; name=\"" << name << "\"\r\n\r\n";
    part << value << "\r\n";
    std::string partStr = part.str();
    parts.emplace_back(partStr.begin(), partStr.end());
}

void MultipartForm::addFile(const std::string& fieldName,
                            const std::string& filename,
                            const std::string& contentType,
                            const std::vector<char>& data)
{
    std::ostringstream part;
    part << "Content-Disposition: form-data; name=\"" << fieldName << "\"; filename=\""
         << filename << "\"\r\n";
    part << "Content-Type: " << contentType << "\r\n\r\n";
    std::string header = part.str();

    std::vector<char> binaryPart;
    binaryPart.reserve(header.size() + data.size() + 2);
    binaryPart.insert(binaryPart.end(), header.begin(), header.end());
    binaryPart.insert(binaryPart.end(), data.begin(), data.end());
    binaryPart.push_back('\r');
    binaryPart.push_back('\n');

    parts.emplace_back(std::move(binaryPart));
}

std::vector<char> MultipartForm::build() const
{
    std::vector<char> body;
    const std::string boundaryPrefix = "--" + boundaryValue + "\r\n";

    for (const auto& part : parts) {
        body.insert(body.end(), boundaryPrefix.begin(), boundaryPrefix.end());
        body.insert(body.end(), part.begin(), part.end());
    }

    std::string closing = "--" + boundaryValue + "--\r\n";
    body.insert(body.end(), closing.begin(), closing.end());

    return body;
}

const std::string& MultipartForm::boundary() const
{
    return boundaryValue;
}
