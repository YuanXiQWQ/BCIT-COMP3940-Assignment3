#pragma once

#include <atomic>
#include <thread>

class Thread {
public:
    Thread();
    virtual ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start();
    void join();
    void detach();
    bool isFinished() const;

protected:
    virtual void run() = 0;

private:
    void entryPoint();

    std::thread worker;
    std::atomic<bool> started;
    std::atomic<bool> finished;
    std::atomic<bool> detached;
};
