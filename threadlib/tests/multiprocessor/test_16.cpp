// Stockers stock more than customers need, so stockers never finish

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

const int VENDING_MACHINE_SIZE = 10;
int vending_machine = 0;
mutex vending_machine_mutex;
cv vending_machine_not_empty;
cv vending_machine_not_full;

const int NUM_STOCKERS = 5;
const int NUM_CUSTOMERS = 5;
const int NUM_TASKS = 20;

int cokes_stocked = 0;
int cokes_vended = 0;

void stocker(uintptr_t id)
{
    for (int i = 0; i < NUM_TASKS; ++i)
    {
        vending_machine_mutex.lock();

        while (vending_machine == VENDING_MACHINE_SIZE)
        {
            vending_machine_not_full.wait(vending_machine_mutex);
        }

        int stock_amount = 1 + (i % 3);
        vending_machine = std::min(vending_machine + stock_amount, VENDING_MACHINE_SIZE);
        cokes_stocked += stock_amount;

        std::cout << "Stocker " << id << " stocked " << stock_amount << " coke(s), " << vending_machine << " total available.\n";

        vending_machine_not_empty.broadcast();
        vending_machine_mutex.unlock();

        thread::yield();
    }
}

void customer(uintptr_t id)
{
    for (int i = 0; i < NUM_TASKS; ++i)
    {
        vending_machine_mutex.lock();

        while (vending_machine == 0)
        {
            vending_machine_not_empty.wait(vending_machine_mutex);
        }

        int vend_amount = 1 + (i % 2);
        vending_machine = std::max(vending_machine - vend_amount, 0);
        cokes_vended += vend_amount;

        std::cout << "Customer " << id << " vended " << vend_amount << " coke(s), " << vending_machine << " remain.\n";

        vending_machine_not_full.broadcast();
        vending_machine_mutex.unlock();

        thread::yield();
    }
}

void test_vending_machine(uintptr_t)
{
    std::cout << "[Multiprocessor] Complex Vending Machine: Running test with 5 stockers and 5 customers.\n";

    std::vector<std::unique_ptr<thread>> threads;
    for (int i = 0; i < NUM_STOCKERS; ++i)
    {
        threads.push_back(std::make_unique<thread>(stocker, i));
    }
    for (int i = 0; i < NUM_CUSTOMERS; ++i)
    {
        threads.push_back(std::make_unique<thread>(customer, i));
    }

    for (auto &t : threads)
    {
        t->join();
    }

    assert(cokes_stocked >= cokes_vended);

    std::cout << "[Multiprocessor] Complex Vending Machine: Test completed.\n";
}

int main()
{
    cpu::boot(5, test_vending_machine, 0, 0, 0, 0);
}
