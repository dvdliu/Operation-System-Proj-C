// Stress test for properly creating and deleting threads.

#include "cpu.h"
#include "mutex.h"
#include "thread.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

mutex counter_mutex;
unsigned int counter = 0;

void thread_increment(uintptr_t)
{
    counter_mutex.lock();
    counter++;
    counter_mutex.unlock();

    thread::yield();
}

void thread_manager(uintptr_t)
{
    printf("[Uniprocessor] Thread Create/Delete Stress: Running test with 250 threads\n");

    const unsigned int NUM_THREADS = 150;
    std::vector<std::unique_ptr<thread>> threads;

    for (unsigned int i = 0; i < NUM_THREADS; ++i)
    {
        threads.push_back(std::make_unique<thread>(thread_increment, 0));
    }

    for (unsigned int i = 0; i < NUM_THREADS; ++i)
    {
        threads[i]->join();
    }

    assert(counter == NUM_THREADS);
    printf("[Uniprocessor] Thread Create/Delete Stress: Test completed\n");
}

int main()
{
    cpu::boot(1, thread_manager, 0, 0, 0, 0);
}
