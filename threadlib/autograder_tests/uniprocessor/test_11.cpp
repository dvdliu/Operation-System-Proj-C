// Reader and writer roles test.

#include "cpu.h"
#include "cv.h"
#include "mutex.h"
#include "thread.h"
#include <iostream>
#include <vector>
#include <memory>

const int NUM_STATS_PPL = 10;
const int NUM_MANAGERS = 30;

mutex rw_mutex;
mutex r_mutex;
cv rw_cv;
cv r_cv;

int viewer_count = 0;
bool update_pending = false;
bool updating = false;
int fantasy_points = 0;

void fantasy_football_manager(uintptr_t id) {
    r_mutex.lock();
    while (update_pending || updating) {
        r_cv.wait(r_mutex);
    }
    ++viewer_count;
    r_mutex.unlock();

    printf("Manager %li reads %u pts\n", id, fantasy_points);
    thread::yield();

    r_mutex.lock();
    --viewer_count;
    if (viewer_count == 0) {
        rw_cv.signal();
    }
    r_mutex.unlock();
}

void espn_stats_person(uintptr_t id) {
    rw_mutex.lock();
    update_pending = true;
    while (viewer_count > 0 || updating) {
        rw_cv.wait(rw_mutex);
    }
    
    updating = true;
    update_pending = false;
    ++fantasy_points;
    printf("Stats person %li updates to %u pts\n", id, fantasy_points);
    thread::yield();

    updating = false;
    rw_cv.signal();
    r_cv.broadcast();
    rw_mutex.unlock();
}

void test_reader_writer(uintptr_t) {
    printf("[Uniprocessor] Fantasy Football: Running test with 10 stats people and 30 managers.\n");

    std::vector<std::unique_ptr<thread>> threads;
    for (int i = 0; i < NUM_STATS_PPL; ++i) {
        threads.push_back(std::make_unique<thread>(espn_stats_person, i));
    }
    for (int i = 0; i < NUM_MANAGERS; ++i) {
        threads.push_back(std::make_unique<thread>(fantasy_football_manager, i));
        thread::yield();
    }

    for (auto& t : threads) {
        t->join();
    }

    printf("[Uniprocessor] Fantasy Football: Test completed.\n");
}

int main() {
    cpu::boot(1, test_reader_writer, 0, 0, 0, 0);
}
