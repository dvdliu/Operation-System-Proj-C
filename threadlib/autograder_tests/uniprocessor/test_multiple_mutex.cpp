#include "thread.h"
#include "mutex.h"
#include "cpu.h"
#include <cassert>
#include <iostream>

mutex mutex1;
mutex mutex2;

int global1 = 0;
int global2 = 0;

int shared = 0;

thread* hello;

void test_func2(uintptr_t)
{
    mutex1.lock();
    int temp = global1;
    hello->join();
    for (int i = 0; i < 20; i++)
    {
        global1 += 1;
    }
    thread::yield();
    shared++;
    assert(global1 - temp == 20);
    std::cout << "[Uniprocessor] Multiple Mutex: Thread 3 finished running" << std::endl;
    mutex1.unlock();
}

void test_func1(uintptr_t)
{
    mutex1.lock();
    for (int i = 0; i < 30; i++)
    {
        global1 += 1;
    }
    thread::yield();

    std::cout << "[Uniprocessor] Multiple Mutex: Thread 2 finished running" << std::endl;
    mutex1.unlock();
}

void test_lock1(uintptr_t)
{
    thread(test_func1, 0);
    thread *thread2 = new thread(test_func2, 0);
    hello->join();
    hello = thread2;
    mutex1.lock();
    int temp = global1;
    for (int i = 0; i < 50; i++)
    {
        global1 += 1;
    }
    thread::yield();
    shared++;
    assert(global1 - temp == 50);
    std::cout << "[Uniprocessor] Multiple Mutex: Thread 1 finished running" << std::endl;
    mutex1.unlock();
}

void test_func3(uintptr_t)
{
    mutex2.lock();
    int temp = global2;
    for (int i = 0; i < 20; i++)
    {
        global2 += 1;
    }
    shared++;
    thread::yield();
    assert(global2 - temp == 20);
    std::cout << "[Uniprocessor] Multiple Mutex: Thread 6 finished running" << std::endl;
    mutex2.unlock();
}

void test_func4(uintptr_t)
{
    mutex2.lock();
    for (int i = 0; i < 30; i++)
    {
        global2 += 1;
    }
    thread::yield();

    std::cout << "[Uniprocessor] Multiple Mutex: Thread 5 finished running" << std::endl;
    mutex2.unlock();
}

void test_lock2(uintptr_t)
{
    thread(test_func3, 0);
    thread(test_func4, 0);
    mutex2.lock();
    int temp = global2;
    for (int i = 0; i < 50; i++)
    {
        global2 += 1;
    }
    shared++;
    thread::yield();
    assert(global2 - temp == 50);
    std::cout << "[Uniprocessor] Multiple Mutex: Thread 4 finished running" << std::endl;
    mutex2.unlock();
}

void test_func(uintptr_t)
{
    std::cout << "[Uniprocessor] Multiple Mutex: Running test with 6 threads" << std::endl;
    thread *thread1 = new thread(test_lock1, 0);
    hello = thread1;
    thread(test_lock2, 0);
}

int main()
{
    cpu::boot(1, test_func, 1, 0, 0, 0);
}