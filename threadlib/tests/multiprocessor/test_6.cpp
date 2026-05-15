// Replaces test_multiple_threads and tests_multiple_threads_yield. Nested locks.

#include "../../cpu.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>

mutex mutex1;
mutex mutex2;
int global_var1 = 0;
int global_var2 = 0;

void test_func3(uintptr_t)
{
    // Nested lock scenario
    mutex2.lock();
    thread::yield();
    mutex1.lock();

    global_var1 += 10;
    global_var2 += 20;

    std::cout << "[Multiprocessor] Test 6: Thread 3 finished running" << std::endl;
    mutex1.unlock();
    mutex2.unlock();
}

void test_func2(uintptr_t)
{
    mutex1.lock();

    thread t3(test_func3, 0);

    global_var1 += 5;
    for (int i = 0; i < 20; i++)
    {
        thread::yield();
    }

    std::cout << "[Multiprocessor] Test 6: Thread 2 finished running" << std::endl;
    mutex1.unlock();

    t3.join();
}

void test_func1(uintptr_t)
{
    thread t2(test_func2, 0);

    // Nested lock
    mutex1.lock();
    mutex2.lock();

    global_var1 += 50;
    global_var2 += 30;

    for (int i = 0; i < 50; i++)
    {
        thread::yield();
    }

    std::cout << "[Multiprocessor] Test 6: Thread 1 finished running" << std::endl;
    mutex2.unlock();
    mutex1.unlock();

    t2.join();
}

void thread_manager(uintptr_t)
{
    std::cout << "[Multiprocessor] Test 6: Running test with 3 threads." << std::endl;

    thread t1(test_func1, 0);
    t1.join();

    assert(global_var1 == 65); // 50 from test_func1, 5 from test_func2, 10 from test_func3
    assert(global_var2 == 50); // 30 from test_func1, 20 from test_func3

    std::cout << "[Multiprocessor] Test 6: All threads finished." << std::endl;
}

int main()
{
    cpu::boot(3, thread_manager, 0, 0, 0, 0);
    return 0;
}
