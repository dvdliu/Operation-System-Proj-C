// Lots of joins inside of other threads

#include "cpu.h"
#include "mutex.h"
#include "thread.h"
#include <iostream>
#include <memory>
#include <vector>

const int NUM_THREADS = 150;
std::vector<std::unique_ptr<thread>> threads;

void worker(uintptr_t id)
{
    printf("Thread %li started.\n", id);
    thread::yield();

    if (id > 0)
    {
        int join_target = (id - 1);
        threads[join_target]->join(); // Join the previous thread
        printf("Thread %li joined thread %d.\n", id, join_target);
    }

    thread::yield();
    printf("Thread %li finished execution.\n", id);
}

void stress_test_join_prev(uintptr_t)
{
    printf("[Uniprocessor] Join Prev: Running test with %d threads.\n", NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.push_back(std::make_unique<thread>(worker, i));
        thread::yield();
    }

    threads[NUM_THREADS - 1]->join();

    printf("[Uniprocessor] Join Prev: Test completed.\n");
}

int main()
{
    cpu::boot(1, stress_test_join_prev, 0, 0, 0, 0);
}
