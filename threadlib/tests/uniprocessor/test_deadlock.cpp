// Nested locks and waiting at different CVs.

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>
#include <vector>
#include <cassert>

thread *hello;

void line_worker(uintptr_t id)
{
    hello->join();
    assert(1 == 2);
}

void test_penn_station(uintptr_t)
{
    printf("[Uniprocessor] Deadlock: Running test with 1 thread.\n");

    hello = new thread(line_worker, 0);
    for(int i = 0; i < 100; i++){
    }
}

int main()
{
    cpu::boot(1, test_penn_station, 0, 1, 1, 1);
}
