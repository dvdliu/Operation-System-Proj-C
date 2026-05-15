#include "cpu.h"
#include "cv.h"
#include "mutex.h"
#include "thread.h"
#include <cassert>
#include <iostream>

mutex mutex1;
mutex mutex2;

cv thread1_cv;
cv thread3_cv;

bool thread_2_finished = false;
bool thread_3_finished = false;
bool thread_4_finished = false;

void test_func1(uintptr_t);
void test_func2(uintptr_t);
void test_func3(uintptr_t);
void test_func4(uintptr_t);
void test_func5(uintptr_t);

void test_func1(uintptr_t)
{
    mutex1.lock();
    int sum = 0;
    for (int i = 0; i < 20; i++)
    {
        sum += 1;
    }
    while (!thread_2_finished)
    {
        thread1_cv.wait(mutex1);
    }
    thread::yield();

    thread thread3 = thread(test_func3, 0);
    thread thread5 = thread(test_func5, 0);

    thread3.join();
    assert(thread_3_finished);
    thread::yield();

    thread5.join();
    ;
    thread::yield();

    assert(sum == 20);
    std::cout << "[Uniprocessor] Multiple Threads: Thread 1 finished running" << std::endl;
    mutex1.unlock();
}

void test_func2(uintptr_t)
{
    mutex2.lock();
    int sum = 0;
    thread::yield();
    for (int i = 0; i < 20; i++)
    {
        sum += 1;
    }
    assert(sum == 20);
    thread::yield();
    std::cout << "[Uniprocessor] Multiple Threads: Thread 2 finished running" << std::endl;
    thread_2_finished = true;

    thread1_cv.signal();
    mutex2.unlock();
}

void test_func3(uintptr_t)
{
    mutex2.lock();
    thread thread4 = thread(test_func4, 0);
    int sum = 0;
    thread::yield();
    for (int i = 0; i < 20; i++)
    {
        sum += 1;
    }
    assert(sum == 20);
    thread4.join();
    thread::yield();
    assert(thread_4_finished);

    std::cout << "[Uniprocessor] Multiple Threads: Thread 3 finished running" << std::endl;
    thread_3_finished = true;
    mutex2.unlock();
}

void test_func4(uintptr_t)
{
    int sum = 0;
    thread::yield();
    for (int i = 0; i < 20; i++)
    {
        sum += 1;
    }
    thread::yield();
    assert(sum == 20);
    std::cout << "[Uniprocessor] Multiple Threads: Thread 4 finished running" << std::endl;
    thread_4_finished = true;
}

void test_func5(uintptr_t)
{
    int sum = 0;
    thread::yield();
    for (int i = 0; i < 90; i++)
    {
        sum += 1;
    }
    thread::yield();
    assert(sum == 90);
    std::cout << "[Uniprocessor] Multiple Threads: Thread 5 finished running" << std::endl;
}

void test(uintptr_t)
{
    std::cout << "[Uniprocessor] Multiple Threads: Running test with 5 threads" << std::endl;
    thread(test_func1, 0);
    thread(test_func2, 0);
}

int main()
{
    cpu::boot(1, test, 1, 0, 0, 0);
}