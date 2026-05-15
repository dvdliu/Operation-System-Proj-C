// Another deadlock test

#include "cpu.h"
#include "mutex.h"
#include "thread.h"
#include <iostream>

mutex mutex1, mutex2, mutex3;

void regular(uintptr_t) {
    mutex1.lock();
    printf("Thread 1 locked mutex 1\n");
    thread::yield();

    mutex2.lock();
    printf("Thread 1 locked mutex 2\n");
    thread::yield();

    mutex3.lock();
    printf("Thread 1 locked mutex 3\n");
    thread::yield();

    mutex3.unlock();
    mutex2.unlock();
    mutex1.unlock();
}

void reverse(uintptr_t) {
    mutex3.lock();
    printf("Thread 2 locked mutex 3\n");
    thread::yield();

    mutex2.lock();
    printf("Thread 2 locked mutex 2\n");
    thread::yield();

    mutex1.lock();
    printf("Thread 2 locked mutex 1\n");
    thread::yield();

    mutex1.unlock();
    mutex2.unlock();
    mutex3.unlock();
}

void mixed(uintptr_t) {
    mutex2.lock();
    printf("Thread 3 locked mutex 2\n");
    thread::yield();

    mutex1.lock();
    printf("Thread 3 locked mutex 1\n");
    thread::yield();

    mutex3.lock();
    printf("Thread 3 locked mutex 3\n");
    thread::yield();

    mutex3.unlock();
    mutex1.unlock();
    mutex2.unlock();
}

void test_deadlock(uintptr_t) {
    printf("[Uniprocessor] Deadlock: Running deadlock test.\n");

    thread t1(regular, 0);
    thread t2(reverse, 0);
    thread t3(mixed, 0);

    t1.join();
    t2.join();
    t3.join();

    printf("[Uniprocessor] Deadlock: Test completed.\n");
}

int main() {
    cpu::boot(1, test_deadlock, 0, 0, 0, 0);
}
