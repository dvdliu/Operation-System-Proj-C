// More complex broadcast

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

const int NUM_WAITERS = 5;
const int NUM_SIGNALERS = 3;
std::vector<bool> thread_has_ran(NUM_WAITERS, false);
mutex mutex1;
cv cv1;

void waiter(uintptr_t id)
{
    mutex1.lock();
    while (!thread_has_ran[id]) {
        cv1.wait(mutex1);
    }
    assert(thread_has_ran[id] == true);
    std::cout << "[Uniprocessor] Single CV: Waiter " << id << " finished running" << std::endl;
    mutex1.unlock();
}

void signaler(uintptr_t id)
{
    thread::yield();
    
    mutex1.lock();
    for (int i = 0; i < NUM_WAITERS; ++i) {
        thread_has_ran[i] = true;
    }
    cv1.broadcast();
    std::cout << "[Uniprocessor] Single CV: Signaler " << id << " finished signaling" << std::endl;
    mutex1.unlock();
}

void thread_manager(uintptr_t)
{
    std::cout << "[Uniprocessor] Single CV: Running test" << std::endl;

    std::vector<std::unique_ptr<thread>> threads;
    for (uintptr_t i = 0; i < NUM_WAITERS; ++i) {
        threads.push_back(std::make_unique<thread>(waiter, i));
    }
    for (uintptr_t i = 0; i < NUM_SIGNALERS; ++i) {
        threads.push_back(std::make_unique<thread>(signaler, i));
    }

    for (auto &t : threads) {
        t->join();
    }

    std::cout << "[Uniprocessor] Single CV: All threads have finished." << std::endl;
}

int main()
{
    cpu::boot(1, thread_manager, 0, 0, 0, 0);
}
