#include "HttpClient.h"
#include "MultipartForm.h"
#include "Url.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

std::string generateBoundary()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return "----ConsoleClientBoundary" + std::to_string(now);
}

std::vector<char> readFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    return std::vector<char>((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

std::string guessContentType(const std::filesystem::path& path)
{
    const std::string ext = path.extension().string();
    if (ext == ".txt") return "text/plain";
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".json") return "application/json";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".zip") return "application/zip";
    return "application/octet-stream";
}

void printUsage()
{
    std::cout << "Usage: upload_client <endpoint-url> <file-path> <caption> <date> [--get]\n"
              << "Example: upload_client http://localhost:8082/ sample.png \"from-cpp-client\" 2025-11-17" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 5) {
        printUsage();
        return EXIT_FAILURE;
    }

    bool fetchForm = false;
    std::vector<std::string> positional;
    positional.reserve(argc - 1);
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--get" || arg == "-g") {
            fetchForm = true;
        } else {
            positional.push_back(std::move(arg));
        }
    }

    if (positional.size() < 4) {
        printUsage();
        return EXIT_FAILURE;
    }

    const std::string endpoint = positional[0];
    const std::filesystem::path filePath = positional[1];
    const std::string caption = positional[2];
    const std::string date = positional[3];

    try {
        Url url(endpoint);
        HttpClient client;

        if (fetchForm) {
            std::cout << "--- Performing GET request to retrieve upload form ---" << std::endl;
            std::string response = client.get(url);
            std::cout << response << std::endl;
        }

        std::vector<char> fileData = readFile(filePath);
        const std::string boundary = generateBoundary();

        MultipartForm form(boundary);
        form.addField("caption", caption);
        form.addField("date", date);
        form.addFile("fileName", filePath.filename().string(), guessContentType(filePath), fileData);

        std::vector<char> body = form.build();
        std::vector<std::string> headers = {
            "Content-Type: multipart/form-data; boundary=" + boundary,
            "Accept: */*"
        };

        std::cout << "--- Performing POST upload to " << endpoint << " ---" << std::endl;
        std::string response = client.post(url, headers, body);
        std::cout << response << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Upload failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
