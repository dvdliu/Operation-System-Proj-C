// Error handling: Release a mutex not being held.

#include "../../cpu.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>

mutex mutex1;
int global = 0;

void thread_1(uintptr_t)
{
    // mutex1.unlock();

    global += 25;

    mutex1.lock();
    printf("[Multiprocessor] Lock w/ Lock: Thread 1 finished running\n");
}

void thread_manager(uintptr_t)
{
    printf("[Multiprocessor] Unlock w/o Lock: Running test with 1 thread\n");

    thread t1(thread_1, 0);
    t1.join();

    assert(global == 25);
    printf("[Multiprocessor] Unlock w/o Lock: Test completed\n");
}

int main()
{
    cpu::boot(5, thread_manager, 0, 0, 0, 0);
}
