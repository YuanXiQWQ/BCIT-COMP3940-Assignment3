#pragma once

#include <filesystem>
#include <string>
#include <vector>

class UploadDirectoryManager {
public:
    static UploadDirectoryManager& instance();

    std::filesystem::path ensureImagesDir();
    std::filesystem::path createTargetFile(const std::string& caption,
                                           const std::string& date,
                                           const std::string& originalName);
    std::vector<std::string> listFilesSorted();

private:
    UploadDirectoryManager() = default;
    std::filesystem::path imagesDir_;
};
