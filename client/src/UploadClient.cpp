#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string currentDate()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
    return buffer;
}

std::string detectWindowsHost()
{
    std::ifstream resolv("/etc/resolv.conf");
    std::string line;
    while(std::getline(resolv, line))
    {
        if(line.empty())
        {
            continue;
        }
        std::istringstream iss(line);
        std::string keyword;
        std::string value;
        if(iss >> keyword >> value)
        {
            if(keyword == "nameserver")
            {
                return value;
            }
        }
    }
    return "127.0.0.1";
}

int connectToHost(const std::string& host, const std::string& port)
{
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res = nullptr;
    int ret = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if(ret != 0)
    {
        throw std::runtime_error(std::string("getaddrinfo failed: ") + gai_strerror(ret));
    }

    int sock = -1;
    for(addrinfo* p = res; p != nullptr; p = p->ai_next)
    {
        sock = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sock < 0)
        {
            continue;
        }
        if(::connect(sock, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;
        }
        ::close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if(sock < 0)
    {
        throw std::runtime_error("Unable to connect to host");
    }
    return sock;
}
}

int main(int argc, char* argv[])
{
    try
    {
        std::string filePath = argc > 1 ? argv[1] : "AndroidLogo.png";
        std::string caption = argc > 2 ? argv[2] : "cpp console";
        std::string date = argc > 3 ? argv[3] : currentDate();
        bool usedAutoHost = argc <= 4;
        std::string host = argc > 4 ? argv[4] : detectWindowsHost();
        int port = argc > 5 ? std::stoi(argv[5]) : 8082;
        std::string path = argc > 6 ? argv[6] : "/";

        std::ifstream file(filePath, std::ios::binary);
        if(!file)
        {
            std::cerr << "File not found: " << filePath << std::endl;
            return 1;
        }
        std::vector<char> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        std::string boundary = "----CascadeBoundary" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::string body;
        auto append = [&](const std::string& part) { body.append(part); };

        append("--" + boundary + "\r\n");
        append("Content-Disposition: form-data; name=\"caption\"\r\n\r\n");
        append(caption + "\r\n");

        append("--" + boundary + "\r\n");
        append("Content-Disposition: form-data; name=\"date\"\r\n\r\n");
        append(date + "\r\n");

        append("--" + boundary + "\r\n");
        append("Content-Disposition: form-data; name=\"fileName\"; filename=\"");
        append(filePath.substr(filePath.find_last_of("/\\") == std::string::npos
                               ? 0
                               : filePath.find_last_of("/\\") + 1));
        append("\"\r\n");
        append("Content-Type: application/octet-stream\r\n\r\n");
        body.append(fileData.data(), fileData.size());
        append("\r\n--" + boundary + "--\r\n");

        std::vector<std::string> hostsToTry{host};
        if(usedAutoHost && host != "127.0.0.1" && host != "localhost")
        {
            hostsToTry.emplace_back("127.0.0.1");
        }

        int sock = -1;
        std::string connectedHost;
        std::string lastErrorMessage;
        for(const auto& candidate : hostsToTry)
        {
            try
            {
                sock = connectToHost(candidate, std::to_string(port));
                connectedHost = candidate;
                break;
            }
            catch(const std::exception& ex)
            {
                lastErrorMessage = ex.what();
            }
        }

        if(sock < 0)
        {
            std::ostringstream error;
            error << "Unable to connect to host; tried ";
            for(std::size_t i = 0; i < hostsToTry.size(); ++i)
            {
                if(i > 0)
                {
                    error << ", ";
                }
                error << hostsToTry[i];
            }
            if(!lastErrorMessage.empty())
            {
                error << ". Last error: " << lastErrorMessage;
            }
            throw std::runtime_error(error.str());
        }

        std::ostringstream requestStream;
        requestStream << "POST " << path << " HTTP/1.1\r\n";
        requestStream << "Host: " << connectedHost << ":" << port << "\r\n";
        requestStream << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";
        requestStream << "Content-Length: " << body.size() << "\r\n";
        requestStream << "Connection: close\r\n\r\n";
        std::string request = requestStream.str();
        request += body;

        std::size_t totalSent = 0;
        while(totalSent < request.size())
        {
            ssize_t n = ::send(sock, request.data() + totalSent, request.size() - totalSent, 0);
            if(n < 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                throw std::runtime_error(std::string("send failed: ") + std::strerror(errno));
            }
            totalSent += static_cast<std::size_t>(n);
        }

        std::string response;
        char buffer[4096];
        ssize_t n;
        while((n = ::recv(sock, buffer, sizeof(buffer), 0)) > 0)
        {
            response.append(buffer, n);
        }
        ::close(sock);

        std::cout << response << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
