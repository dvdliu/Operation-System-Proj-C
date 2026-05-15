// Thread must acquire multiple shared resources. Deadlocks are likely.

#include "../../cpu.h"
#include "../../cv.h"
#include "../../mutex.h"
#include "../../thread.h"
#include <iostream>
#include <memory>
#include <vector>

const int NUM_INGREDIENTS = 3;
const int NUM_BAKERS = 5;

std::vector<mutex> ingredient_mutexes(NUM_INGREDIENTS);
std::vector<cv> ingredient_cvs(NUM_INGREDIENTS);
std::vector<bool> ingredient_available(NUM_INGREDIENTS, true);

void baker(uintptr_t id)
{
    int first_ingredient = id % NUM_INGREDIENTS;
    int second_ingredient = (id + 1) % NUM_INGREDIENTS;

    // Attempt to acquire first ingredient
    ingredient_mutexes[first_ingredient].lock();
    while (!ingredient_available[first_ingredient])
    {
        ingredient_cvs[first_ingredient].wait(ingredient_mutexes[first_ingredient]);
    }
    ingredient_available[first_ingredient] = false;
    std::cout << "Baker " << id << " acquired ingredient " << first_ingredient << std::endl;
    ingredient_mutexes[first_ingredient].unlock();

    // Simulate work
    thread::yield();

    // Attempt to acquire second ingredient
    ingredient_mutexes[second_ingredient].lock();
    while (!ingredient_available[second_ingredient])
    {
        ingredient_cvs[second_ingredient].wait(ingredient_mutexes[second_ingredient]);
    }
    ingredient_available[second_ingredient] = false;
    std::cout << "Baker " << id << " acquired ingredient " << second_ingredient << std::endl;
    ingredient_mutexes[second_ingredient].unlock();

    // Simulate work with both ingredients
    thread::yield();
    std::cout << "Baker " << id << " is working with ingredients "
              << first_ingredient << " and " << second_ingredient << std::endl;

    // Give up second ingredient
    ingredient_mutexes[second_ingredient].lock();
    ingredient_available[second_ingredient] = true;
    ingredient_cvs[second_ingredient].signal();
    std::cout << "Baker " << id << " released ingredient " << second_ingredient << std::endl;
    ingredient_mutexes[second_ingredient].unlock();

    // Give up first ingredient
    ingredient_mutexes[first_ingredient].lock();
    ingredient_available[first_ingredient] = true;
    ingredient_cvs[first_ingredient].signal();
    std::cout << "Baker " << id << " released ingredient " << first_ingredient << std::endl;
    ingredient_mutexes[first_ingredient].unlock();
}

void test_bake_off(uintptr_t)
{
    std::cout << "[Uniprocessor] Running bake off test with 5 threads and 3 shared resources." << std::endl;

    std::vector<std::unique_ptr<thread>> bakers;
    for (uintptr_t i = 0; i < NUM_BAKERS; ++i)
    {
        bakers.push_back(std::make_unique<thread>(baker, i));
    }

    for (auto &baker_thread : bakers)
    {
        baker_thread->join();
    }

    std::cout << "[Uniprocessor] Bake off test completed." << std::endl;
}

int main()
{
    cpu::boot(1, test_bake_off, 0, 0, 0, 0);
    return 0;
}
