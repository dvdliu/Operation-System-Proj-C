/*
 * mutex.h -- interface to the mutex class
 *
 * You may add new variables and functions to this class.
 *
 * Do not modify any of the given function declarations.
 */

#pragma once

#include <memory>
#include <queue>
#include <ucontext.h>

using ctx_ptr = std::unique_ptr<ucontext_t>;
using std::queue;

class mutex {
public:
    mutex();
    ~mutex();

    void lock();
    void unlock();

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    /*
     * Move constructor and move assignment operator.  Implementing these is
     * optional in Project 2.
     */
    mutex(mutex&&);
    mutex& operator=(mutex&&);

    // Newly added members and methods

    void unlock_mutex();
    std::weak_ptr<thread_info> owned_thread;
    queue<thread_ptr> waiting_threads;     // queue of threads waiting for
                                        // this lock

    bool free;                          // status of lock
};