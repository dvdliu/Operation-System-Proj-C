#include "cpu.h"
#include "thread.h"
#include <iostream>
#include <memory>

std::unique_ptr<thread> t;

void self_joiner(uintptr_t)
{
    printf("Thread is running.\n");
    t->join();
    printf("Thread finished.\n");
}

void test_double_join(uintptr_t)
{
    printf("[Uniprocessor] Join Self: Running test\n");
    t = std::make_unique<thread>(self_joiner, 0);
    thread::yield();
    t->join();
    printf("[Uniprocessor] Join Self: Test completed\n");
}

int main()
{
    cpu::boot(1, test_double_join, 0, 0, 0, 0);
}
