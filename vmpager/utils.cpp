#include "utils.h"
#include <cassert>
#include <cstring>

// Define global variables
deque<shared_ptr<mem_block>> clock_queue;
pid_t curr_pid;
queue<unsigned int> free_mem_blocks;
queue<unsigned int> free_swap_blocks;
unordered_map<unsigned int, shared_ptr<kernel_process_info>> process_info_mp;
shared_ptr<mem_block> zero_page = std::make_shared<mem_block>(0, block_type::phys);
unordered_map<pair<string, unsigned int>, shared_ptr<mem_block>, FileBlockHash> file_mapping_table;
vector<char> tmp_buffer(VM_PAGESIZE);

// kernel page info ctor
kernel_page_info::kernel_page_info() : valid(false) {}

// kernel process info ctor
kernel_process_info::kernel_process_info() : page_info_table(array<kernel_page_info, num_virt_addr>()), next_invalid_page(0) {
    page_table_entry_t default_entry = {0, 0, 0};
    std::fill(page_table.begin(), page_table.end(), default_entry);
}

// mem block ctor
mem_block::mem_block(unsigned int location_in, block_type type_in) : dirty(false), referenced(true), resident(true),
    location(location_in), type(type_in) {
        if(location == 0 && block_type::phys == type) {
            is_swap = true;
        }
    }


/* * * * * * * * * * * * * * * Helper Function Definitions * * * * * * * * * * * * * * * * * * */


unsigned int addr_to_index(const void* addr) {
    return (static_cast<const char *>(addr) - static_cast<char *>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;
}

void *index_to_addr(unsigned int index) {
    return reinterpret_cast<void *>(static_cast<char *>(VM_ARENA_BASEADDR) + index * VM_PAGESIZE);
}

unsigned int calculate_page_offset(const void* addr) {
    uintptr_t address = reinterpret_cast<uintptr_t>(addr);
    return address % VM_PAGESIZE; // Offset within the page
}

pair<int, char> get_data(const void* addr, kernel_page_info *page_info) {
    unsigned int page_offset = calculate_page_offset(addr);
    const char *physmem_ptr = static_cast<char *>(vm_physmem);
    unsigned int virtual_page = addr_to_index(addr);

    // Bring in filename block if it is not resident
    shared_ptr<mem_block> filename_mem_block = page_info[virtual_page].data_location;
    int success = 0;
    if(!filename_mem_block->resident) {
        success = cache_nonresident_page(filename_mem_block);
    }
    if(success == 0) {return std::make_pair(success, physmem_ptr[page_info[virtual_page].data_location->location * VM_PAGESIZE + page_offset]); }
    return std::make_pair(success, 'a');
}

int cache_nonresident_page(shared_ptr<mem_block> &faulting_mem_block) {
    // Ensure freed memory
    assert(!faulting_mem_block->resident);
    assert(faulting_mem_block->type != block_type::phys);
    if(free_mem_blocks.empty()) {
        evict_phys_page();
    }

    unsigned int updated_mem_location = free_mem_blocks.front();
    unsigned int file_location = faulting_mem_block->location;

    // Get filename and file block depending on if it is a swap or file backed page
    const char* filename = faulting_mem_block->is_swap ? nullptr: faulting_mem_block->filename.c_str();
    int success = file_read(filename, file_location, static_cast<char*>(vm_physmem) + (VM_PAGESIZE * updated_mem_location));
    if(success == 0) {
        free_mem_blocks.pop();
        // Page is now resident, so it is elegible for eviction
        clock_queue.push_back(faulting_mem_block);
        faulting_mem_block->type = block_type::phys;
        faulting_mem_block->referenced = true;
        faulting_mem_block->resident = true;
        faulting_mem_block->dirty = false;
        faulting_mem_block->location = updated_mem_location;

        // Update ptes referencing this mem_block
        for(page_table_entry_t *ptbr: faulting_mem_block->ptes) {
            ptbr->ppage = updated_mem_location;
            ptbr->read_enable = 1;
            ptbr->write_enable = 0;
        }
    }
    return success;
}

void read_fault_handler(shared_ptr<mem_block> &faulting_mem_block){
    // If reading and the data is dirty, mark as write enabled
    if(faulting_mem_block->dirty && ((faulting_mem_block->is_swap && faulting_mem_block->ptes.size() == 1) ||
    !faulting_mem_block->is_swap)) {
        for (page_table_entry_t *ptbr: faulting_mem_block->ptes) {
        ptbr->write_enable = 1;
        }
    }

    // Newly allocated memory blocks should be referenced and read-enabled
    faulting_mem_block->referenced = true;
    faulting_mem_block->resident = true;
    for (page_table_entry_t *ptbr: faulting_mem_block->ptes) {
        ptbr->read_enable = 1;
    }
}

void write_fault_handler(shared_ptr<mem_block> &faulting_mem_block) {
    // Allow future writes and mark data as dirty
    for (page_table_entry_t *ptbr: faulting_mem_block->ptes) {
    ptbr->write_enable = 1;
    }
    faulting_mem_block->dirty = true;
}

void update_block_permissions(shared_ptr<mem_block> &faulting_mem_block) {
    if(faulting_mem_block->dirty && faulting_mem_block->ptes.size() == 1 && faulting_mem_block->referenced) {
        (*faulting_mem_block->ptes.begin())->write_enable = 1;
        (*faulting_mem_block->ptes.begin())->read_enable = 1;
    }
}

void copy_block_handler(kernel_page_info &faulting_page_info, page_table_entry_t *faulting_pte) {
    shared_ptr<mem_block> old_block = faulting_page_info.data_location;
    // Create a copy to write to
    shared_ptr<mem_block> allocated_mem_block = std::make_shared<mem_block>(free_mem_blocks.front(), block_type::phys);
    faulting_page_info.data_location = allocated_mem_block;
    allocated_mem_block->is_swap = true;

    // Allocate a swap block to the new memory from the old referenced memory
    allocated_mem_block->swap_blocks.push_back(old_block->swap_blocks.back());
    old_block->swap_blocks.pop_back();

    // Update mem block information
    allocated_mem_block->ptes.insert(faulting_pte);
    faulting_pte->ppage = allocated_mem_block->location;
    free_mem_blocks.pop();

    // Add the newly created block onto the eviction queue
    clock_queue.push_back(faulting_page_info.data_location);

    // Sanity check and state update
    allocated_mem_block->dirty = true;
    allocated_mem_block->resident = true;
    allocated_mem_block->referenced = true;

    void* new_phys_addr = static_cast<char*>(vm_physmem) + (allocated_mem_block->location * VM_PAGESIZE);
    memcpy(new_phys_addr, tmp_buffer.data(), VM_PAGESIZE);
}

void copy_virtual_page(shared_ptr<kernel_process_info> &parent_info, shared_ptr<kernel_process_info> &child_info, unsigned int page_num) {
    // Mark the child pages as valid and update the ppages for the page table
    child_info->page_info_table[page_num].valid = true;
    child_info->page_table[page_num].ppage = parent_info->page_table[page_num].ppage;

    // Parent and child process point to the same memory
    shared_ptr<mem_block> shared_mem_block = parent_info->page_info_table[page_num].data_location;
    child_info->page_info_table[page_num].data_location = shared_mem_block;

    // Add child pte to memory block, and copy over read enable bit
    shared_mem_block->ptes.insert(&child_info->page_table[page_num]);
    child_info->page_table[page_num].read_enable = parent_info->page_table[page_num].read_enable;

    if(shared_mem_block->is_swap) { // swap-backed pages
        // Reserve a spot on the swap file for the copied swap file
        child_info->page_info_table[page_num].data_location->swap_blocks.push_back(free_swap_blocks.front());
        free_swap_blocks.pop();

        if(shared_mem_block == zero_page) return; // Sanity check for pinned page

        // Set the write bit to 0 for both processes, to ensure copy-on-write behavior
        for(page_table_entry_t *ptbr: shared_mem_block->ptes) {
            ptbr->write_enable = 0;
        }
    } else { // file-backed pages
        // Copy parent write bit
        child_info->page_table[page_num].write_enable = parent_info->page_table[page_num].write_enable;
    }
}

void map_swap_block(kernel_process_info *process_info, unsigned int new_page) {
    page_table_base_register[new_page].read_enable = 1;
    page_table_base_register[new_page].write_enable = 0;

    auto &kernel_info_table = process_info->page_info_table;

    // Read from the zero page, and reserve a swap block
    kernel_info_table[new_page].data_location = zero_page;
    zero_page->swap_blocks.push_back(free_swap_blocks.front());
    kernel_info_table[new_page].data_location->is_swap = true;
    process_info->num_swap_blocks++;
    free_swap_blocks.pop();

    page_table_base_register[new_page].ppage = 0;
}

void inspect_clock_queue(unordered_set<unsigned int> &useless_mem_blocks) {
    useless_mem_blocks.insert(0u);  // Do not add the 0 page to the clock queue
    size_t clock_size = clock_queue.size();
    for(size_t i = 0; i < clock_size; i++) {
        if(useless_mem_blocks.find(clock_queue.front()->location) == useless_mem_blocks.end()) {
            clock_queue.push_back(clock_queue.front());
        }
        clock_queue.pop_front();
    }
}

pair<int, string> read_filename_str(const char* filename) {
    string filename_str;
    const char* str_ptr = filename;
    auto &kernel_info_table = process_info_mp[curr_pid]->page_info_table;

    if(reinterpret_cast<uintptr_t>(filename) < reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) return std::make_pair(-1, "a");

    // Get the filename string from the application address space
    while(true) {
        unsigned int virtual_page = addr_to_index(str_ptr);
        if(virtual_page >= num_virt_addr || !kernel_info_table[virtual_page].valid) {
            // Exit if the string is not in the application address space
            return std::make_pair(-1, "a");
        }
        auto [success, next_char] = get_data(str_ptr, &kernel_info_table[0]);
        if(success == -1) return std::make_pair(-1, "a");
        if (next_char == '\0') return std::make_pair(0, filename_str);

        filename_str += next_char;
        str_ptr++;
    }
}

void map_file_block(string filename_str, unsigned int block, unsigned int new_page) {
    shared_ptr<mem_block> new_file_block = std::make_shared<mem_block>(block, block_type::file);
    pair<string, unsigned int> file_map_key = std::make_pair(filename_str, block);

    auto &kernel_info_table = process_info_mp[curr_pid]->page_info_table;
    // Set state for the file mem block struct
    kernel_info_table[new_page].data_location = new_file_block;
    new_file_block->is_swap = false;
    new_file_block->resident = false;
    new_file_block->referenced = false;
    new_file_block->block_num = block;
    new_file_block->filename = filename_str;

    page_table_base_register[new_page].ppage = new_file_block->location;

    file_mapping_table[file_map_key] = new_file_block;
    new_file_block->ptes.insert(&page_table_base_register[new_page]);

    page_table_base_register[new_page].read_enable = 0;
    page_table_base_register[new_page].write_enable = 0;
}

void update_file_block(shared_ptr<mem_block> &used_file_block, unsigned int new_page) {
    auto &kernel_info_table = process_info_mp[curr_pid]->page_info_table;

    if(used_file_block->resident) {
        page_table_base_register[new_page].ppage = used_file_block->location;
    }
    if(!used_file_block->ptes.empty()) {
        page_table_entry_t *reference_pte = *used_file_block->ptes.begin();
        page_table_base_register[new_page].read_enable = reference_pte->read_enable;
        page_table_base_register[new_page].write_enable = reference_pte->write_enable;
        page_table_base_register[new_page].ppage = reference_pte->ppage;
    } else {
        // If the block of data exists in memory and is referenced, read enable is 1
        page_table_base_register[new_page].read_enable = 0;
        page_table_base_register[new_page].write_enable = 0;
        if(used_file_block->resident && used_file_block->referenced){
            page_table_base_register[new_page].read_enable = 1;
            if(used_file_block->dirty) {
                page_table_base_register[new_page].write_enable = 1;
            }
        }
    }
    used_file_block->ptes.insert(&page_table_base_register[new_page]);

    kernel_info_table[new_page].data_location = used_file_block;
}

bool is_copy_on_writable(shared_ptr<mem_block> &faulting_mem_block) {
    return (faulting_mem_block->location == 0 || faulting_mem_block->ptes.size() > 1) && faulting_mem_block->is_swap;
}

/* * * * * * * * * * * * * * * Eviction Algorithm * * * * * * * * * * * * * * * * * * */

void evict_phys_page() {
    // Find an unreferenced virtual address using a physical page
    assert(clock_queue.front()->type == block_type::phys);

    // Find an unreferenced page
    while(clock_queue.front()->referenced) {
        shared_ptr<mem_block> eviction_candidate = clock_queue.front();

        // Mark all ptes are read_enable false and write_enable false
        for(auto i: eviction_candidate->ptes) {
            i->read_enable = 0;
            i->write_enable = 0;
        }

        eviction_candidate->referenced = 0;

        // 2nd chance
        clock_queue.push_back(eviction_candidate);
        clock_queue.pop_front();
    }

    auto evicted_mem_block = clock_queue.front();
    clock_queue.pop_front();

    for(auto i: evicted_mem_block->ptes) {
        i->read_enable = 0;
        i->write_enable = 0;
    }

    // Add free the physical memory this block is holding
    unsigned int freed_mem = evicted_mem_block->location;
    assert(evicted_mem_block->type == block_type::phys);
    free_mem_blocks.push(freed_mem);

    bool should_write = evicted_mem_block->dirty;

    // Preemptively set state if this block gets read in
    evicted_mem_block->resident = 0;
    evicted_mem_block->dirty = 0;
    evicted_mem_block->referenced = 0;

    unsigned int evicted_mem_page = evicted_mem_block->location;
    const char* filename;
    unsigned int file_block;

    // Update the evicted block information to where it is being sent:
    if(!evicted_mem_block->is_swap) { // To a file
        evicted_mem_block->type = block_type::file;
        filename = evicted_mem_block->filename.c_str();
        file_block = evicted_mem_block->block_num;
        evicted_mem_block->location = evicted_mem_block->block_num;

        if(evicted_mem_block->ptes.empty()) {
            auto evicted_file_info = std::make_pair(evicted_mem_block->filename, evicted_mem_block->block_num);
            file_mapping_table.erase(evicted_file_info);
        }
    } else {
        evicted_mem_block->type = block_type::swap;
        filename = nullptr;
        file_block = evicted_mem_block->swap_blocks.front();
        evicted_mem_block->location = evicted_mem_block->swap_blocks.front();
    }

    // Only write to the disk if the data is dirty
    if(should_write) {
        file_write(filename, file_block, static_cast<char*>(vm_physmem) + (VM_PAGESIZE * evicted_mem_page));
    }
}