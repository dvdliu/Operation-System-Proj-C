// Based on vending machine coke example from class

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <cassert>
#include <iostream>

const int VENDING_MACHINE_SIZE = 10;
int vending_machine = 0;
mutex vending_machine_mutex;
cv vending_machine_not_empty;
cv vending_machine_not_full;
int cokes_stocked = 0;
int cokes_vended = 0;

void stocker(uintptr_t id)
{
    for (int i = 0; i < 20; ++i) // Each stocker stocks 20 cokes
    {
        vending_machine_mutex.lock();

        while (vending_machine == VENDING_MACHINE_SIZE)
        {
            vending_machine_not_full.wait(vending_machine_mutex);
        }

        ++vending_machine;
        ++cokes_stocked;
        std::cout << "Stocker " << id << " stocked a coke, " << vending_machine << " available" << std::endl;

        vending_machine_not_empty.signal();

        vending_machine_mutex.unlock();

        thread::yield();
    }
}

void customer(uintptr_t id)
{
    for (int i = 0; i < 20; ++i) // Each customer consumes 20 cokes
    {
        vending_machine_mutex.lock();

        while (vending_machine == 0)
        {
            vending_machine_not_empty.wait(vending_machine_mutex);
        }

        --vending_machine;
        ++cokes_vended;
        std::cout << "Customer " << id << " vended a coke, " << vending_machine << " remain" << std::endl;

        vending_machine_not_full.signal();

        vending_machine_mutex.unlock();

        thread::yield();
    }
}

void test_vending_machine(uintptr_t)
{
    std::cout << "[Uniprocessor] Running vending machine test with 2 stockers and 2 customers." << std::endl;
    thread t1(stocker, 1);
    thread t2(stocker, 2);
    thread t3(customer, 1);
    thread t4(customer, 2);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    assert(cokes_stocked == cokes_vended);
    std::cout << "[Uniprocessor] Vending machine test completed." << std::endl;
}

int main()
{
    cpu::boot(1, test_vending_machine, 0, 0, 0, 0);
    return 0;
}
