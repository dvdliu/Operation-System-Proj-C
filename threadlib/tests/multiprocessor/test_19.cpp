#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <cassert>
#include <iostream>

bool thread3_has_ran = false;
mutex mutex1;
cv cv1;

void thread1(uintptr_t)
{
    thread::yield();
    mutex1.lock();
    while (!thread3_has_ran)
    {
        cv1.wait(mutex1);
    }
    assert(thread3_has_ran == true);
    printf("[Multiprocessor] Signals: Thread 1 finished running\n");
    mutex1.unlock();
}

void thread2(uintptr_t)
{
    thread::yield();
    mutex1.lock();
    while (!thread3_has_ran)
    {
        cv1.wait(mutex1);
    }
    assert(thread3_has_ran == true);
    printf("[Multiprocessor] Signals: Thread 2 finished running\n");
    mutex1.unlock();
}

void thread3(uintptr_t)
{
    thread::yield();
    mutex1.lock();
    thread3_has_ran = true;
    cv1.signal();
    cv1.signal();
    mutex1.unlock();

    printf("[Multiprocessor] Signals: Thread 3 finished running\n");
}

void thread4(uintptr_t)
{
    thread::yield();
    mutex1.lock();
    while (!thread3_has_ran)
    {
        cv1.wait(mutex1);
    }
    assert(thread3_has_ran == true);
    printf("[Multiprocessor] Signals: Thread 4 finished running\n");
    mutex1.unlock();
}

void thread_manager(uintptr_t)
{
    printf("[Multiprocessor] Signals: Running test with 3 waiting threads\n");
    thread(thread1, 0);
    thread(thread2, 0);
    thread(thread3, 0);
    thread(thread4, 0);
}

int main()
{
    cpu::boot(5, thread_manager, 0, 0, 0, 0);
}
