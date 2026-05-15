#include "../../thread.h"
#include "../../cpu.h"
#include <iostream>

bool finished_thread1 = false;

void test_thread1(uintptr_t){
    int sum = 0;
    for(int i = 0; i < 50; i++){
        sum += 1;
    }
    // thread::yield();
    assert(sum == 50);
    finished_thread1 = true;
    std::cout << "[Multiprocessor] Simple Join: Thread 1 finished running" << std::endl;
}

void test_thread2(uintptr_t){
    thread thread1 = thread(test_thread1, 0);
    int sum = 0;
    for(int i = 0; i < 50; i++){
        sum += 1;
    }

    // thread::yield();

    thread1.join();
    assert(sum == 50);
    assert(finished_thread1);
    std::cout << "[Multiprocessor] Simple Join: Thread 2 finished running" << std::endl;
}

void test_func(uintptr_t){
    std::cout << "[Multiprocessor] Simple Join: Running test with 2 threads" << std::endl;
    thread(test_thread2, 0);
}


int main(){
    cpu::boot(2, test_thread2, 1, 0, 0, 0);
}