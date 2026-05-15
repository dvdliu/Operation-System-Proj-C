#include "../../thread.h"
#include "../../cpu.h"
#include <cassert>
#include <iostream>

void test_func3(uintptr_t)
{
    int sum = 0;
    for (int i = 0; i < 20; i++)
    {
        sum += 1;
    }
    assert(sum == 20);
    std::cout << "[Multiprocessor] Multiple Threads: Thread 3 finished running" << std::endl;
}

void test_func2(uintptr_t)
{
    // printf("Starting thread 2\n");
    thread(test_func3, 0);
    int sum = 0;
    for (int i = 0; i < 20; i++)
    {
        // printf("What\n");
        sum += 1;
    }
    assert(sum == 20);
    // printf("Done with thread 2\n");
    std::cout << "[Multiprocessor] Multiple Threads: Thread 2 finished running" << std::endl;
}

void test_func1(uintptr_t)
{
    std::cout << "[Multiprocessor] Multiple Threads: Running test with 3 threads" << std::endl;
    thread(test_func2, 0);
    int sum = 0;
    for (int i = 0; i < 50; i++)
    {
        sum += 1;
    }
    assert(sum == 50);
    std::cout << "[Multiprocessor] Multiple Threads: Thread 1 finished running" << std::endl;
}

int main()
{
    cpu::boot(5, test_func1, 1, 0, 0, 0);
}