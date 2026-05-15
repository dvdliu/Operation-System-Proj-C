// Lots of threads, limited resources. Lots of mutexes and CVs.

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>
#include <memory>
#include <vector>

const int NUM_GAMERS = 100;
const int NUM_COMPUTERS = 10;

std::vector<mutex> computer_mutexes(NUM_COMPUTERS);
std::vector<cv> computer_cvs(NUM_COMPUTERS);
std::vector<bool> computer_available(NUM_COMPUTERS, true);

void gamer(uintptr_t id)
{
    int computer = id % NUM_COMPUTERS;

    // Attempt to acquire a computer
    computer_mutexes[computer].lock();
    while (!computer_available[computer])
    {
        computer_cvs[computer].wait(computer_mutexes[computer]);
    }
    computer_available[computer] = false;
    std::cout << "Gamer " << id << " acquired computer " << computer << std::endl;
    computer_mutexes[computer].unlock();

    // Simulate gamig on the computer
    thread::yield();

    // Give up the computer
    computer_mutexes[computer].lock();
    computer_available[computer] = true;
    computer_cvs[computer].signal();
    std::cout << "Gamer " << id << " released computer " << computer << std::endl;
    computer_mutexes[computer].unlock();
}

void test_computer_contention(uintptr_t)
{
    std::cout << "[Uniprocessor] Running computer contention test with 100 threads and 10 shared resources." << std::endl;

    std::vector<std::unique_ptr<thread>> gamers;
    for (uintptr_t i = 0; i < NUM_GAMERS; ++i)
    {
        gamers.push_back(std::make_unique<thread>(gamer, i));
    }

    for (auto &gamer_thread : gamers)
    {
        gamer_thread->join();
    }

    std::cout << "[Uniprocessor] Computer contention test completed." << std::endl;
}

int main()
{
    cpu::boot(1, test_computer_contention, 0, 0, 0, 0);
}
