#pragma once

#include <string>
#include <vector>

class MultipartForm {
public:
    explicit MultipartForm(std::string boundary);

    void addField(const std::string& name, const std::string& value);
    void addFile(const std::string& fieldName,
                 const std::string& filename,
                 const std::string& contentType,
                 const std::vector<char>& data);

    std::vector<char> build() const;
    const std::string& boundary() const;

private:
    std::string boundaryValue;
    std::vector<std::vector<char>> parts;
};
