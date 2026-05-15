#include "../../thread.h"
#include "../../mutex.h"
#include "../../cpu.h"
#include <cassert>
#include <iostream>

mutex mutex1;
int global = 0;

void test_func3(uintptr_t) {
    mutex1.lock();
    int initial_global = global;
    for (int i = 0; i < 20; i++) {
        global += 1;
    }

    thread::yield();

    assert(global - initial_global == 20);
    mutex1.unlock();

    printf("[Multiprocessor] Single Mutex: Thread 3 finished running\n");
}

void test_func2(uintptr_t) {
    mutex1.lock();
    thread t3(test_func3, 0);

    thread::yield();

    int initial_global = global;
    for (int i = 0; i < 30; i++) {
        global += 1;
    }
    assert(global - initial_global == 30);
    
    mutex1.unlock();
    t3.join();

    printf("[Multiprocessor] Single Mutex: Thread 2 finished\n");
}

void test_func1(uintptr_t) {
    printf("[Multiprocessor] Single Mutex: Running test with 3 threads\n");
    thread t2(test_func2, 0);

    mutex1.lock();
    int initial_global = global;

    thread::yield();

    for (int i = 0; i < 50; i++) {
        global += 1;
    }
    assert(global - initial_global == 50);

    mutex1.unlock();
    t2.join();

    printf("[Multiprocessor] Single Mutex: Thread 1 finished running\n");
}

int main() {
    cpu::boot(5, test_func1, 1, 0, 0, 0);
}
