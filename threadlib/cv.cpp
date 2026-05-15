#include <cassert>
#include <memory>
#include "cpu.h"
#include "cv.h"
#include "os_guard.h"
#include "utils.h"

cv::cv(): waiting_threads(queue<thread_ptr>()) {}

void cv::wait(mutex &curr_mutex)
{
    {
        os_guard guard;
        curr_mutex.unlock_mutex();

        // Get the current thread
        thread_ptr curr_thread = std::move(cpu::self()->executing_thread);
        std::weak_ptr<thread_info> curr_thread_ptr = curr_thread;

        // Add current thread to threads waiting in this cv
        this->waiting_threads.push(std::move(curr_thread));

        // Get the next ready thread
        wait_and_pop(curr_thread_ptr);
    }
    curr_mutex.lock();
}

void cv::signal()
{
    os_guard guard;

    if (!this->waiting_threads.empty())
    {
        // Get a waiting thread from the waiting queue
        thread_ptr waiting_thread = std::move(this->waiting_threads.front());
        ucontext_t *waiting_thread_ptr = waiting_thread->thread_ctx.get();
        assert(waiting_thread_ptr != nullptr);
        this->waiting_threads.pop();

        // Add the waiting thread to the ready queue
        push_and_signal(std::move(waiting_thread));
    }
}

void cv::broadcast()
{
    os_guard guard;

    // Move all waiting threads to the ready queue
    while (!this->waiting_threads.empty())
    {
        thread_ptr waiting_thread = std::move(this->waiting_threads.front());
        ucontext_t *waiting_thread_ptr = waiting_thread->thread_ctx.get();
        assert(waiting_thread_ptr != nullptr);
        this->waiting_threads.pop();

        push_and_signal(std::move(waiting_thread));
    }

    assert(this->waiting_threads.empty());
}

cv::~cv() {}