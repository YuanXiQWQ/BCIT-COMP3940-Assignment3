#include "ServerSocket.hpp"
#include "UploadServerThread.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

namespace {
volatile std::sig_atomic_t g_running = 1;

void signalHandler(int)
{
    g_running = 0;
}
}

int main()
{
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try
    {
        ServerSocket server(8082);
        std::cout << "UploadServer listening on port 8082" << std::endl;

        std::vector<std::thread> workers;
        while(g_running)
        {
            try
            {
                Socket client = server.accept();
                workers.emplace_back(UploadServerThread(std::move(client)));
            }
            catch(const std::exception& ex)
            {
                std::cerr << "Accept failed: " << ex.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        for(auto& worker : workers)
        {
            if(worker.joinable())
            {
                worker.join();
            }
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
