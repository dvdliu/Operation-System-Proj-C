#include <memory>
#include <ucontext.h>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "thread.h"
#include "cpu.h"
#include "os_guard.h"
#include "utils.h"

using std::make_shared;

void thread::func_with_swap(thread_startfunc_t func, uintptr_t arg)
{
    // Enable interrupts and disable guard
    cpu::guard.store(false);
    cpu::interrupt_enable();

    // Run the user code
    func(arg);
    {
        os_guard guard;

        // Delete previously finished thread stacks
        while(!cpu::finished_threads.empty()) cpu::finished_threads.pop();

        // Check if there are any threads waiting for this thread to finish
        auto &waiting_threads_queue = cpu::self()->executing_thread->threads_waiting;

        // Add threads that are waiting for this tread onto ready queue
        while(!waiting_threads_queue.empty()){
            thread_ptr next_waiting_thread = waiting_threads_queue.front();
            waiting_threads_queue.pop();
            push_and_signal(std::move(next_waiting_thread));
        }

        // Hand off thread stack for next finished thread
        cpu::self()->executing_thread->finished = true;
        thread_ptr curr_thread = std::move(cpu::self()->executing_thread);
        cpu::self()->executing_thread = nullptr;
        cpu::finished_threads.push(std::move(curr_thread));

        // Get the next thread
        wait_and_pop(std::weak_ptr<thread_info>());
    }
}


thread::thread(thread_startfunc_t func, uintptr_t arg)
{
    os_guard guard;
    thread_ptr curr_thread = create_thread(func, arg);
    this->info = curr_thread;
    push_and_signal(std::move(curr_thread));
}

void thread::yield()
{
    os_guard guard;

    if(cpu::self()->executing_thread == cpu::self()->waiting_thread) return;

    if(!cpu::ready_threads.empty()){
        // Push executing thread onto the ready queue
        ucontext_t *curr_thread_ptr = cpu::self()->executing_thread.get()->thread_ctx.get();
        cpu::ready_threads.push(std::move(cpu::self()->executing_thread));

        // Get the next ready thread
        cpu::self()->executing_thread = std::move(cpu::ready_threads.front());
        ucontext_t *next_thread_ptr = cpu::self()->executing_thread->thread_ctx.get();
        assert(next_thread_ptr != nullptr);
        cpu::ready_threads.pop();

        // Swap the current thread and next ready thread
        swapcontext(curr_thread_ptr, next_thread_ptr);
    }
}

void thread::join()
{
    os_guard guard;

    {if(!this->info.lock() || this->info.lock()->finished) return;}

    thread_ptr curr_thread = std::move(cpu::self()->executing_thread);
    std::weak_ptr<thread_info> curr_thread_ptr = curr_thread;

    {this->info.lock()->threads_waiting.push(curr_thread); }

    // Get the next ready thread
    wait_and_pop(curr_thread_ptr);
}

thread::~thread() {}