#include "Thread.h"

#include <stdexcept>

Thread::Thread()
    : worker(), started(false), finished(false), detached(false)
{
}

Thread::~Thread()
{
    if (started && !detached && worker.joinable()) {
        worker.join();
    }
}

void Thread::start()
{
    bool expected = false;
    if (!started.compare_exchange_strong(expected, true)) {
        throw std::runtime_error("Thread already started");
    }
    worker = std::thread(&Thread::entryPoint, this);
}

void Thread::join()
{
    if (!started) {
        return;
    }
    if (worker.joinable()) {
        worker.join();
        finished = true;
    }
}

void Thread::detach()
{
    if (!started) {
        return;
    }
    if (!detached.exchange(true) && worker.joinable()) {
        worker.detach();
    }
}

bool Thread::isFinished() const
{
    return finished;
}

void Thread::entryPoint()
{
    try {
        run();
    } catch (...) {
        // swallow exceptions to avoid std::terminate; extend with logging if needed
    }
    finished = true;
}
