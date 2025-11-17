#include "ServerSocket.h"

#include <stdexcept>
#include <cstring>
#include <memory>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

ServerSocket::ServerSocket(int port){
#ifdef _WIN32
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        throw std::runtime_error("WSAStartup failed");
    }
#endif

    sock = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if(sock < 0){
        throw std::runtime_error("opening stream socket");
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(static_cast<uint16_t>(port));

    int opt = 1;
#ifdef _WIN32
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt),
               sizeof(opt));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    if(bind(sock, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0){
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        ::close(sock);
#endif
        throw std::runtime_error("binding stream socket");
    }

    if(listen(sock, 16) < 0){
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        ::close(sock);
#endif
        throw std::runtime_error("listen failed");
    }
}

std::unique_ptr<Socket> ServerSocket::Accept(){
    sockaddr_in remoteAddr{};
#ifdef _WIN32
    int addrLen = sizeof(remoteAddr);
#else
    socklen_t addrLen = sizeof(remoteAddr);
#endif

    int client = static_cast<int>(accept(sock, reinterpret_cast<sockaddr *>(&remoteAddr),
                                         &addrLen));
    if(client < 0){
        throw std::runtime_error("accept failed");
    }

    return std::make_unique<Socket>(client);
}

ServerSocket::~ServerSocket(){
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    ::close(sock);
#endif
}
