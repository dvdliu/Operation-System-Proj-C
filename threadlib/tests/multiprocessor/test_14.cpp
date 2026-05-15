#include "../../cpu.h"
#include "../../thread.h"
#include <iostream>

void double_join_victim(uintptr_t)
{
    printf("Thread is running.\n");
    thread::yield();
    printf("Thread finished.\n");
}

void test_double_join(uintptr_t)
{
    printf("[Multiprocessor] Double Join: Running test\n");
    thread t(double_join_victim, 0);
    t.join();

    printf("Attempting double join...\n");
    t.join(); // This should cause an error or be handled gracefully.
    printf("[Multiprocessor] Double Join: Test completed\n");
}

int main()
{
    cpu::boot(5, test_double_join, 0, 0, 0, 0);
}
