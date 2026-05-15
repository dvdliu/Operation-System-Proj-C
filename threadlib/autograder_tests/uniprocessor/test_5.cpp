// Replaces test_single_mutex. Frequent yielding.

#include "cpu.h"
#include "mutex.h"
#include "thread.h"
#include <cassert>
#include <iostream>

mutex sum_mutex;
int global_sum = 0;

void thread3(uintptr_t)
{
    int local_sum = 0;
    for (int i = 0; i < 20; i++)
    {
        local_sum += 1;
        thread::yield();
    }

    sum_mutex.lock();
    global_sum += local_sum;
    sum_mutex.unlock();

    assert(local_sum == 20);
    std::cout << "[Uniprocessor] Test 5: Thread 3 finished running" << std::endl;
}

void thread2(uintptr_t)
{
    thread t3(thread3, 0);

    int local_sum = 0;
    for (int i = 0; i < 20; i++)
    {
        local_sum += 1;
        thread::yield();
    }

    sum_mutex.lock();
    global_sum += local_sum;
    sum_mutex.unlock();

    t3.join(); // Wait for test_func3 to complete

    assert(local_sum == 20);
    std::cout << "[Uniprocessor] Test 5: Thread 2 finished running" << std::endl;
}

void thread1(uintptr_t)
{
    thread t2(thread2, 0);

    int local_sum = 0;
    for (int i = 0; i < 50; i++)
    {
        local_sum += 1;
        if (i % 10 == 0)
            thread::yield();
    }

    sum_mutex.lock();
    global_sum += local_sum;
    sum_mutex.unlock();

    t2.join(); // Wait for test_func2 to complete

    assert(local_sum == 50);
    std::cout << "[Uniprocessor] Test 5: Thread 1 finished running" << std::endl;
}

void thread_manager(uintptr_t)
{
    std::cout << "[Uniprocessor] Test 5: Running test with 3 threads" << std::endl;

    thread t1(thread1, 0);
    t1.join();

    assert(global_sum == (50 + 20 + 20)); // 50 from test_func1, 20 from test_func2, 20 from test_func3
    std::cout << "[Uniprocessor] Test 5: All threads finished." << std::endl;
}

int main()
{
    cpu::boot(1, thread_manager, 0, 0, 0, 0);
    return 0;
}
