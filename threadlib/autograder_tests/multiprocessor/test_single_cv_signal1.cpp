#include "thread.h"
#include "mutex.h"
#include "cpu.h"
#include "cv.h"
#include <cassert>
#include <iostream>

bool thread3_has_ran = false;
mutex mutex1;
cv cv1;

void thread1(uintptr_t) {
    mutex1.lock();
    while (!thread3_has_ran) {
        cv1.wait(mutex1);
    }
    assert(thread3_has_ran == true);
    std::cout << "[Uniprocessor] Single CV: Thread 1 finished running" << std::endl;
    mutex1.unlock();
}

void thread2(uintptr_t) {
    mutex1.lock();
    while (!thread3_has_ran) {
        cv1.wait(mutex1);
    }
    assert(thread3_has_ran == true);
    std::cout << "[Uniprocessor] Single CV: Thread 2 finished running" << std::endl;
    mutex1.unlock();
}

void thread3(uintptr_t) {
    mutex1.lock();
    thread3_has_ran = true;
    cv1.signal();
    std::cout << "[Uniprocessor] Single CV: Thread 3 finished running" << std::endl;
    mutex1.unlock();
}

void thread_manager(uintptr_t) {
    std::cout << "[Uniprocessor] Single CV: Running test with 2 threads" << std::endl;
    thread(thread1, 0);
    thread(thread2, 0);
    thread(thread3, 0);
}

int main() {
    cpu::boot(3, thread_manager, 0, 0 ,0, 0);
}
