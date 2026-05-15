#include <cassert>
#include <memory>
#include <stdexcept>
#include <ucontext.h>
#include "cpu.h"
#include "mutex.h"
#include "os_guard.h"
#include "utils.h"

mutex::mutex() : waiting_threads(queue<thread_ptr>()), free(true) {}

void mutex::lock(){
    os_guard guard;

    if(!this->free){
        // Get the current thread
        thread_ptr curr_thread = std::move(cpu::self()->executing_thread);
        std::weak_ptr<thread_info> curr_thread_ptr = curr_thread;

        // Add current thread to threads waiting for this lock
        this->waiting_threads.push(std::move(curr_thread));

        // Get the next ready thread
        wait_and_pop(curr_thread_ptr);
    } else {
        // Set status to busy and stay on the current thread
        this->free = false;
        this->owned_thread = cpu::self()->executing_thread;
    }

    assert(!this->free);
}

void mutex::unlock(){
    os_guard guard;

    unlock_mutex();
}

void mutex::unlock_mutex() {
    if(this->free || cpu::self()->executing_thread != this->owned_thread.lock()){
        throw std::runtime_error("mutex::unlock called without holding the lock");
    }

    this->owned_thread.reset();
    this->free = true;      // Free the lock

    if(!this->waiting_threads.empty()){
        // Get first waiting thread to guarantee FIFO ordering
        assert(!this->waiting_threads.empty());
        thread_ptr unlocked_thread = std::move(this->waiting_threads.front());
        this->waiting_threads.pop();

        this->owned_thread = unlocked_thread;
        this->free = false;
        // Add waiting thread to ready queue, and preemptively lock the mutex
        push_and_signal(std::move(unlocked_thread));
    }
}

mutex::~mutex(){}