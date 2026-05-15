#pragma once

#include <cassert>
#include <memory>
#include <queue>
#include <ucontext.h>

struct thread_info
{
    std::unique_ptr<ucontext_t> thread_ctx;
    std::unique_ptr<char[]> stack_mem;
    std::queue<std::shared_ptr<thread_info>> threads_waiting;
    bool finished;

    thread_info(std::unique_ptr<ucontext_t> &&ctx_in);
};