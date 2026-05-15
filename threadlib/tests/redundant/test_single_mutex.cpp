#include "../../thread.h"
#include "../../mutex.h"
#include "../../cpu.h"
#include <cassert>
#include <iostream>

mutex mutex1;

int global = 0;

void test_func3(uintptr_t)
{
    mutex1.lock();
    int temp = global;
    for (int i = 0; i < 20; i++)
    {
        global += 1;
    }
    thread::yield();
    assert(global - temp == 20);
    std::cout << "[Uniprocessor] Single Mutex: Thread 3 finished running" << std::endl;
    mutex1.unlock();
}

void test_func2(uintptr_t)
{
    mutex1.lock();
    thread(test_func3, 0);

    thread::yield();
    for (int i = 0; i < 30; i++)
    {
        global += 1;
    }

    std::cout << "[Uniprocessor] Single Mutex: Thread 2 finished running" << std::endl;
    mutex1.unlock();
}

void test_func1(uintptr_t)
{
    std::cout << "[Uniprocessor] Single Mutex: Running test with 3 threads" << std::endl;
    thread(test_func2, 0);
    mutex1.lock();
    int temp = global;
    thread::yield();
    for (int i = 0; i < 50; i++)
    {
        global += 1;
    }
    assert(global - temp == 50);
    std::cout << "[Uniprocessor] Single Mutex: Thread 1 finished running" << std::endl;
    mutex1.unlock();
}

int main()
{
    cpu::boot(1, test_func1, 1, 0, 0, 0);
}