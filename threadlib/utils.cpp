#include <cassert>
#include <memory>
#include <ucontext.h>
#include <cstdio>
#include "cpu.h"
#include "utils.h"
#include "thread.h"

using std::make_shared;


void push_and_signal(thread_ptr &&waiting_thread){
    // Add a thread to the ready queue
    cpu::ready_threads.push(std::move(waiting_thread));

    // Signal a suspended cpu to wake
    if(!cpu::suspended_cpus.empty()){
        cpu *cpu_to_wake = cpu::suspended_cpus.front();
        cpu::suspended_cpus.pop();
        cpu_to_wake->interrupt_send();
    }
}

void wait_for_ready_thread() {
    while(true){
        // Suspend cpu while waiting
        while(cpu::ready_threads.empty()){
            cpu::suspended_cpus.push(cpu::self());
            cpu::guard.store(false);
            cpu::interrupt_enable_suspend();

            cpu::interrupt_disable();
            while(cpu::guard.exchange(true)){}
        }

        ucontext_t *curr_thread_ptr = cpu::self()->waiting_thread->thread_ctx.get();
        cpu::self()->executing_thread = std::move(cpu::ready_threads.front());
        ucontext_t *next_thread_ptr = cpu::self()->executing_thread->thread_ctx.get();
        cpu::ready_threads.pop();

        swapcontext(curr_thread_ptr, next_thread_ptr);
    }
}

void wait_and_pop(std::weak_ptr<thread_info> curr_thread_ptr){
    thread_ptr new_thread;

    if(cpu::ready_threads.empty()){
        new_thread = cpu::self()->waiting_thread;
    } else {
        // Pop a thread from the ready queue to return
        new_thread = std::move(cpu::ready_threads.front());
        cpu::ready_threads.pop();
    }

    assert(new_thread);
    ucontext_t *created_thread = new_thread->thread_ctx.get();

    if (curr_thread_ptr.lock()) {
        cpu::self()->executing_thread = std::move(new_thread);
        ucontext_t *curr_thread_ctx = curr_thread_ptr.lock()->thread_ctx.get();
        if(curr_thread_ctx != created_thread){
            swapcontext(curr_thread_ctx, created_thread);
        }
    } else {
        cpu::self()->executing_thread = std::move(new_thread);
        setcontext(created_thread);
    }
}


thread_ptr create_thread(thread_startfunc_t func, uintptr_t arg){
    // Initialize thread context
    ctx_ptr ucontext_ptr = std::make_unique<ucontext_t>();
    ucontext_t *created_thread = ucontext_ptr.get();

    created_thread->uc_stack.ss_size = STACK_SIZE;
    created_thread->uc_stack.ss_flags = 0;
    created_thread->uc_link = nullptr;
    thread_ptr new_thread = std::make_shared<thread_info>(std::move(ucontext_ptr));
    makecontext(created_thread, reinterpret_cast<void (*)()>(thread::func_with_swap), 2, func, arg);

    assert(new_thread->thread_ctx.get() != nullptr);

    // Add newly created thread to the queue of ready threads
    thread_ptr thread_obj_ptr = new_thread;
    return thread_obj_ptr;
}