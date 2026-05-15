#include "../../thread.h"
#include "../../cpu.h"
#include <cassert>
#include <iostream>

void thread_a(uintptr_t){
    int sum = 0;
    for(int i = 0; i < 50; i++){
        sum += 1;
    }
    thread::yield();
    assert(sum == 50);
    std::cout << "[Uniprocessor] Multiple Threads Yield: Thread 1 Finished" << std::endl;
}

void thread_b(uintptr_t){
    int sum = 0;
    for(int i = 0; i < 50; i++){
        sum += 1;
    }
    thread::yield();
    assert(sum == 50);
    std::cout << "[Uniprocessor] Multiple Threads Yield: Thread 2 Finished" << std::endl;
}

void test_func(uintptr_t){
    std::cout << "[Uniprocessor] Multiple Threads: Running test with 2 threads" << std::endl;
    thread(thread_a, 0);
    thread(thread_b, 0);
}


int main(){
    cpu::boot(1, test_func, 1, 0, 0, 0);
}