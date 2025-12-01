#include "UploadDirectoryManager.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace {
std::string sanitize(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    for(char ch : input)
    {
        if(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-' || ch == '.')
        {
            result.push_back(ch);
        }
        else
        {
            result.push_back('_');
        }
    }
    if(result.empty())
    {
        result = "unknown";
    }
    return result;
}
}

UploadDirectoryManager& UploadDirectoryManager::instance()
{
    static UploadDirectoryManager INST;
    return INST;
}

std::filesystem::path UploadDirectoryManager::ensureImagesDir()
{
    if(imagesDir_.empty())
    {
        imagesDir_ = std::filesystem::current_path() / "images";
    }
    if(!std::filesystem::exists(imagesDir_))
    {
        std::filesystem::create_directories(imagesDir_);
    }
    return imagesDir_;
}

std::filesystem::path UploadDirectoryManager::createTargetFile(const std::string& caption,
                                                               const std::string& date,
                                                               const std::string& originalName)
{
    auto dir = ensureImagesDir();
    std::string safeCaption = sanitize(caption);
    std::string safeDate = sanitize(date);
    std::string safeName = sanitize(originalName);
    if(safeName.empty())
    {
        safeName = "upload.bin";
    }

    std::ostringstream oss;
    if(!safeCaption.empty())
    {
        oss << safeCaption << '_';
    }
    if(!safeDate.empty())
    {
        oss << safeDate << '_';
    }
    oss << safeName;

    std::filesystem::path candidate = dir / oss.str();
    if(std::filesystem::exists(candidate))
    {
        auto now = std::chrono::system_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        candidate = dir / (oss.str() + "_" + std::to_string(millis));
    }
    return candidate;
}

std::vector<std::string> UploadDirectoryManager::listFilesSorted()
{
    auto dir = ensureImagesDir();
    std::vector<std::string> files;
    if(std::filesystem::exists(dir))
    {
        for(const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if(entry.is_regular_file())
            {
                files.push_back(entry.path().filename().string());
            }
        }
    }
    std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
        std::string al = a;
        std::string bl = b;
        std::transform(al.begin(), al.end(), al.begin(), ::tolower);
        std::transform(bl.begin(), bl.end(), bl.begin(), ::tolower);
        return al < bl;
    });
    return files;
}
