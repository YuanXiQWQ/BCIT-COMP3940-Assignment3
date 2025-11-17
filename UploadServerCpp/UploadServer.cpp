#include "ServerSocket.h"
#include "UploadServerThread.h"
#include "UploadServlet.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

int main(int argc, char* argv[])
{
    int port = 8082;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port argument, defaulting to 8082" << std::endl;
        }
    }

    try {
        ServerSocket server(port);
        UploadServlet servlet;
        std::vector<std::unique_ptr<UploadServerThread>> workers;

        std::cout << "UploadServer (C++) listening on port " << port << "..." << std::endl;

        while (true) {
            try {
                auto client = server.Accept();
                auto worker = std::make_unique<UploadServerThread>(std::move(client), servlet);
                worker->start();
                workers.emplace_back(std::move(worker));

                workers.erase(std::remove_if(workers.begin(), workers.end(), [](const auto& thread) {
                    return thread->isFinished();
                }), workers.end());
            } catch (const std::exception& ex) {
                std::cerr << "Exception while handling client: " << ex.what() << std::endl;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Failed to start server: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
