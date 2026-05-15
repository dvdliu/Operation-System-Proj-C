#pragma once

#include <cassert>
#include <memory>
#include <ucontext.h>
#include "cpu.h"

using thread_ptr = std::shared_ptr<thread_info>;

/**
 * @brief Push a thread context onto the ready queue, and
 *        signal a suspended cpu to wake
 * 
 * @param waiting_thread - thread to push onto the queue
 */
void push_and_signal(thread_ptr &&waiting_thread);

/**
 * @brief Wait and get the next ready thread
 * 
 * @param curr_thread_ptr reference to the current running context
 */
void wait_and_pop(std::weak_ptr<thread_info> curr_thread_ptr);

/**
 * @brief Create a thread object
 * 
 * @param func 
 * @param arg 
 * @return thread_ptr 
 */
thread_ptr create_thread(thread_startfunc_t func, uintptr_t arg);

/**
 * @brief Function for cpu to wait for ready threads
 * 
 */
void wait_for_ready_thread();