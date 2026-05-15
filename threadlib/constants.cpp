#include <cassert>
#include <memory>
#include <queue>
#include <ucontext.h>
#include "constants.h"
#include "thread.h"

thread_info::thread_info(std::unique_ptr<ucontext_t> &&ctx_in) : thread_ctx(std::move(ctx_in)), stack_mem(std::make_unique<char[]>(STACK_SIZE)), finished(false)
{
    thread_ctx.get()->uc_stack.ss_sp = stack_mem.get();
};