# Operating Systems Project

A mock operating system built in C++, implementing core OS internals from the ground up. What started as a class operating systems assignment grew into a broader systems project touching **scheduling**, **memory management**, and **filesystem design**.

```
┌─────────────────────────────────────────────────────────┐
│                      testapp                             │
│           (sample programs exercising the OS)            │
└───────────────┬───────────────────┬───────────────────┬──┘
                │                   │                   │
        ┌───────▼───────┐   ┌───────▼───────┐   ┌───────▼───────┐
        │   threadlib   │   │   vmpager     │   │   filesys     │
        │  (scheduling) │   │   (memory)    │   │   (storage)   │
        └───────┬───────┘   └───────┬───────┘   └───────┬───────┘
                │                   │                   │
                └───────────────────┴───────────────────┘
                              Kernel core
```

---

## 🧵 `threadlib` — Custom Thread Library

A user-space threading implementation that handles concurrent execution without relying on the OS's native thread support.

- **Context switching** — saves and restores register state to swap execution between threads
- **Scheduling** — decides which thread runs next (e.g. round-robin / priority-based scheduling logic)
- **Synchronization primitives** — locks, mutexes, or condition variables to coordinate shared state across threads
- **Thread lifecycle management** — creation, blocking, yielding, and termination of threads

This module is the foundation the rest of the OS builds on — filesystem and paging operations all execute in the context of threads managed here.

---

## 🧠 `vmpager` — Virtual Memory Pager

Implements virtual memory, translating between the address space a program sees and physical memory underneath.

- **Page table management** — maps virtual addresses to physical frames
- **Page fault handling** — intercepts invalid memory accesses and resolves them by loading pages in
- **Paging / eviction policy** — decides what stays resident in memory and what gets swapped out under pressure
- **Address translation** — the core virtual → physical lookup path

This is the piece most directly tied to the "memory management" focus of the project — it's what lets `testapp` programs behave as if they have more memory than physically exists.

---

## 💾 `filesys` — Networked File System

A filesystem implementation with network support, handling how data is organized, stored, and retrieved.

- **File & directory abstractions** — the on-disk (or in-memory) structures representing files and directories
- **Storage layout** — how data blocks are allocated and tracked
- **Network support** — allows filesystem operations to work across a network rather than purely locally
- **Read/write operations** — the actual I/O path client code goes through to interact with files

---

## 🧪 `testapp` — Test Applications

Sample programs used to exercise and validate the other three modules — spinning up threads, triggering page faults, and reading/writing through the filesystem to confirm everything behaves correctly together.

---

## Build

Build instructions depend on the provided Makefiles in each module:

```bash
make
```

Each subdirectory (`threadlib/`, `vmpager/`, `filesys/`, `testapp/`) can typically be built independently, or from the root to build everything at once.

## Background

This project began as a class operating systems assignment and was expanded afterward into a more complete systems exploration — going beyond the minimum requirements to dig deeper into how real OS components like schedulers, virtual memory managers, and filesystems fit together.
