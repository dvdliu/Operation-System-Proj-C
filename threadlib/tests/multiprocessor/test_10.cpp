// Nested locks and waiting at different CVs.

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>
#include <vector>

const int NUM_THREADS = 50;

mutex bread_mutex, meat_mutex, cheese_mutex;
cv bread_cv, meat_cv, cheese_cv;

unsigned int bread_available = 12;
unsigned int meat_available = 9;
unsigned int cheese_available = 6;

void line_worker(uintptr_t id)
{
    bread_mutex.lock();
    while (bread_available == 0)
    {
        bread_cv.wait(bread_mutex);
    }
    --bread_available;
    printf("Thread %li used bread\n", id);

    thread::yield();

    meat_mutex.lock();
    while (meat_available == 0)
    {
        meat_cv.wait(meat_mutex);
    }
    --meat_available;
    printf("Thread %li used meat\n", id);

    thread::yield();

    cheese_mutex.lock();
    while (cheese_available == 0)
    {
        cheese_cv.wait(cheese_mutex);
    }
    --cheese_available;
    printf("Thread %li used cheese\n", id);

    bread_mutex.unlock();
    meat_mutex.unlock();
    cheese_mutex.unlock();

    thread::yield();

    cheese_mutex.lock();
    ++cheese_available;
    cheese_cv.signal();
    printf("Thread %li stocked cheese\n", id);
    cheese_mutex.unlock();

    thread::yield();

    meat_mutex.lock();
    ++meat_available;
    meat_cv.signal();
    printf("Thread %li stocked meat\n", id);
    meat_mutex.unlock();

    thread::yield();

    bread_mutex.lock();
    ++bread_available;
    bread_cv.signal();
    printf("Thread %li stocked bread\n", id);
    bread_mutex.unlock();
}

void test_penn_station(uintptr_t)
{
    printf("[Multiprocessor] Penn Station: Running test with 50 threads.\n");

    std::vector<std::unique_ptr<thread>> threads;
    for (uintptr_t i = 0; i < NUM_THREADS; ++i)
    {
        threads.push_back(std::make_unique<thread>(line_worker, i));
    }

    for (auto &t : threads)
    {
        t->join();
    }

    assert(bread_available == 12);
    assert(meat_available == 9);
    assert(cheese_available == 6);

    printf("[Multiprocessor] Penn Station: Test completed.\n");
}

int main()
{
    cpu::boot(5, test_penn_station, 0, 0, 0, 0);
}
