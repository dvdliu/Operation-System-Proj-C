#include <ucontext.h>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "os_guard.h"
#include "utils.h"

queue<thread_ptr> cpu::ready_threads;
queue<thread_ptr> cpu::finished_threads;
queue<cpu *>  cpu::suspended_cpus;

void timer_interrupt_handler(){
    thread::yield();
}

void ipi_interrupt_handler(){}


cpu::cpu(thread_startfunc_t func, uintptr_t arg) : executing_thread(nullptr)
{
    try {
        while(cpu::guard.exchange(true)) {}
        this->interrupt_vector_table[TIMER] = reinterpret_cast<void (*)()>(timer_interrupt_handler);
        this->interrupt_vector_table[IPI] = reinterpret_cast<void (*)()>(ipi_interrupt_handler);

        ctx_ptr ucontext_ptr = std::make_unique<ucontext_t>();
        ucontext_t *created_thread = ucontext_ptr.get();

        created_thread->uc_stack.ss_size = STACK_SIZE;
        created_thread->uc_stack.ss_flags = 0;
        created_thread->uc_link = nullptr;

        this->waiting_thread = std::make_shared<thread_info>(std::move(ucontext_ptr));
        makecontext(created_thread, wait_for_ready_thread, 0);

        // Create main thread on cpu
        if(func){
            this->executing_thread = create_thread(func, arg);
            ucontext_t *curr_thread_ptr = this->executing_thread->thread_ctx.get();
            setcontext(curr_thread_ptr);
        } else {
            wait_and_pop(std::weak_ptr<thread_info>());
        }
    } catch (std::bad_alloc& e) {
        cpu::guard.store(false);
        interrupt_enable();
        throw;
    }
}
