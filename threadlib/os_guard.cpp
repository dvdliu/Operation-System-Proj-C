#include <ucontext.h>
#include <atomic>
#include "os_guard.h"
#include "cpu.h"

os_guard::os_guard() {
    // Disable interrupts on creation
    cpu::interrupt_disable();

    // Busy wait for guard on this cpu
    while (cpu::guard.exchange(true)){}
}

os_guard::~os_guard(){
     // Allow access for other cpus
    cpu::guard.store(false);

    // Enable interrupts on destruction
    cpu::interrupt_enable();
}