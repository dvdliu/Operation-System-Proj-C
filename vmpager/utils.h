
#pragma once

#include "vm_arena.h"
#include "vm_pager.h"
#include <queue>
#include <unordered_map>
#include <array>
#include <vector>
#include <unordered_set>
#include <memory>
#include <utility>
#include <string>
#include <iostream>
#include <deque>

using std::queue;
using std::vector;
using std::array;
using std::unordered_map;
using std::unordered_set;
using std::unique_ptr;
using std::shared_ptr;
using std::pair;
using std::string;
using std::deque;

constexpr uint64_t num_virt_addr = (VM_ARENA_SIZE / VM_PAGESIZE);

/* * * * * * * * * * * * Data Structures * * * * * * * * * * * * */

// Enum clas to hold a mem_block type
enum class block_type {
    phys = 0,
    swap = 1,
    file = 2
};

/*
 * Abstraction for a physical block of memory
 * Can and should be shared among virtual pages
 */
struct mem_block {
    bool dirty;
    bool referenced;
    bool resident;
    bool is_swap;

    deque<int> swap_blocks;                     // reserved swap blocks, specific for swap-backed pages
    unordered_set<page_table_entry_t *> ptes;   // dependent page_table_entries

    unsigned int location;                      // page where the address points to
    block_type type;                            // location of the page

    // specific for file-backed pages
    string filename;
    unsigned int block_num;

    mem_block(unsigned int location_in, block_type type_in);
};

// Implementation for the pinned 0 page
extern shared_ptr<mem_block> zero_page;

/*
 * Page info for kernel to keep track of for faulting behavior
 */
struct kernel_page_info {
    bool valid;
    std::shared_ptr<mem_block> data_location;

    kernel_page_info();
};


/*
 * Info stored for each process in the kernel
 * 
 * - kernel_table: table of page info for each virtual address
 * - page_table: data structure for mmu to access
 * - next_invalid_page: the next invalid page that the process can use
 */
struct kernel_process_info {
    array<kernel_page_info, num_virt_addr> page_info_table;
    array<page_table_entry_t, num_virt_addr> page_table;
    unsigned int next_invalid_page;
    unsigned int num_swap_blocks;
    // Maybe add more for 6-credit

    kernel_process_info();
};


/*
 * Map of pids to a page_info_table to keep track of each
 * process's information
 */
extern unordered_map<unsigned int, shared_ptr<kernel_process_info>> process_info_mp;

/*
 * Queue of physical memory blocks that are unused, or invalid
 * 
 * Processes should pop from this queue when being created, or
 * mapping to a new address, and push when freeing up a page
 */
extern queue<unsigned int> free_mem_blocks;


/*
 * Queue of memory blocks that live in physical memory, eligable for eviction
 * 
 * - For the clock algorithm to evict a page
 * - An invariant should be that size = phys_mem blocks when evict is called
 */
extern deque<shared_ptr<mem_block>> clock_queue;

/*
 * Queue of swap blocks that are free to use
 * 
 * The size should be checked when processes are created for
 * eager swap reservation, and indices should be added/removed
 * as swap blocks are used or freed
 */
extern queue<unsigned int> free_swap_blocks;

/*
 * Process currently running on the CPU
 */
extern pid_t curr_pid;


// Custom hash function for storing file information
struct FileBlockHash {
    inline size_t operator()(const std::pair<std::string, unsigned int>& key) const {
        size_t hash1 = std::hash<std::string>()(key.first);
        size_t hash2 = std::hash<unsigned int>()(key.second);
        return hash1 ^ (hash2 << 1);
    }
};


/*
 * map of (filename, block) to PageInfo to keep track of which filename/block pairs we already are referencing.
 * this ensures all virtual pages mapping the same file block share the same physical page or swap block.
*/
extern unordered_map<pair<string, unsigned int>, shared_ptr<mem_block>, FileBlockHash> file_mapping_table;

extern std::vector<char> tmp_buffer;

/* * * * * * * * * * * * Helper Functions * * * * * * * * * * * * */

/**
 * @brief Converts a virtual address to an index into the page table
 * 
 * @param addr 
 * @return unsigned int 
 */
unsigned int addr_to_index(const void* addr);

/**
 * @brief Converts an index into the page table into a virtual address
 * 
 * @param index 
 * @return void* 
 */
void *index_to_addr(unsigned int index);

/**
 * @brief Read the name of the function lol
 * 
 * @param addr 
 * @return unsigned int 
 */
unsigned int calculate_page_offset(const void* addr);

/**
 * @brief Retrieve data from phys_mem given a virtual address and page info
 * 
 * @param addr 
 * @param page_info 
 * @return char 
 */
pair<int, char> get_data(const void* addr, kernel_page_info *page_info);

/**
 * @brief Move a memory block into physical memory
 * 
 * @param faulting_mem_block 
 */
int cache_nonresident_page(shared_ptr<mem_block> &faulting_mem_block);


/**
 * @brief Update information for block that is being read
 * 
 * @param faulting_mem_block 
 */
void read_fault_handler(shared_ptr<mem_block> &faulting_mem_block);

/**
 * @brief Update information for a block that is being written
 * 
 * @param faulting_mem_block 
 */
void write_fault_handler(shared_ptr<mem_block> &faulting_mem_block);

/**
 * @brief Reinspect block to update page table entry
 * 
 * @param faulting_mem_block 
 */
void update_block_permissions(shared_ptr<mem_block> &faulting_mem_block);

/**
 * @brief Copy a block and update the ptbr entry and mem_block
 * 
 * @param faulting_page_info 
 * @param faulting_pte 
 */
void copy_block_handler(kernel_page_info &faulting_page_info, page_table_entry_t *faulting_pte);

/**
 * @brief Copy a parent virtual page onto a child virtual page and update accordingly
 * 
 * @param parent_info 
 * @param child_info 
 * @param page_num 
 */
void copy_virtual_page(shared_ptr<kernel_process_info> &parent_info, shared_ptr<kernel_process_info> &child_info, unsigned int page_num);

/**
 * @brief Update the ptbr and create a reference to the zero page
 * 
 * @param process_info 
 * @param new_page 
 */
void map_swap_block(kernel_process_info *process_info, unsigned int new_page);

/**
 * @brief Repopulate the clock queue with useful data
 * 
 * @param useless_mem_blocks 
 */
void inspect_clock_queue(unordered_set<unsigned int> &useless_mem_blocks);

/**
 * @brief Read in a filename string, returns success and string
 * 
 * @param filename 
 * @return pair<int, string> 
 */
pair<int, string> read_filename_str(const char* filename);

/**
 * @brief Create a new mem block that lives in a file, update ptbr
 * 
 * @param filename_str 
 * @param block 
 * @param new_page 
 */
void map_file_block(string filename_str, unsigned int block, unsigned int new_page);

/**
 * @brief Update a file block and ptbr for when it is mapped
 * 
 * @param used_file_block 
 * @param new_page 
 */
void update_file_block(shared_ptr<mem_block> &used_file_block, unsigned int new_page);

/**
 * @brief Check a block to see if it qualifies for copy on write
 * 
 * @param faulting_mem_block 
 * @return true 
 * @return false 
 */
bool is_copy_on_writable(shared_ptr<mem_block> &faulting_mem_block);


/**
 * @brief Selects a block in physical memory to remove and pushes it onto the free memory queue
 * 
 */
void evict_phys_page();

