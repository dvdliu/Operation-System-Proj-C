#include "../../thread.h"
#include "../../cpu.h"
#include <cassert>
#include <iostream>

void test_func(uintptr_t){
    int sum = 0;
    for(int i = 0; i < 50; i++){
        sum += 1;
    }
    thread::yield();
    assert(sum == 50);
    std::cout << "[Uniprocessor] Single Thread Yield: PASSED" << std::endl;
}


int main(){
    cpu::boot(1, test_func, 1, 0, 0, 0);
}