#include "vm_pager.h"
#include "utils.h"
#include <cassert>
#include <cstring>

void vm_init(unsigned int memory_pages, unsigned int swap_blocks) {
    // Push all memory_pages into free_phys_blocks, and swap blocks into free_swap_blocks
    for (unsigned int i = 1; i < memory_pages; i++) // Start at i = 1 since 0 is pinned
        free_mem_blocks.push(i);

    for (unsigned int i = 0; i < swap_blocks; i++)
        free_swap_blocks.push(i);
    assert(free_swap_blocks.size() == swap_blocks);

    memset(vm_physmem, 0, VM_PAGESIZE);
}

int vm_create(pid_t parent_pid, pid_t child_pid) {
    // Create empty child arena
    process_info_mp[child_pid] = std::make_shared<kernel_process_info>();

    // Copy parent arena if the kernel manages the parent process
    if (process_info_mp.find(parent_pid) != process_info_mp.end()) {

        shared_ptr<kernel_process_info> parent_info = process_info_mp[parent_pid];

        // Reserve as many swap blocks as the parent has for the child process
        if(parent_info->num_swap_blocks > free_swap_blocks.size()) return -1;

        // Copy over number of parent swap blocks to child swap blocks
        shared_ptr<kernel_process_info> child_info = process_info_mp[child_pid];
        child_info->num_swap_blocks = parent_info->num_swap_blocks;

        // Copy the valid parent memory to the child
        child_info->next_invalid_page = parent_info->next_invalid_page;
        for(unsigned int i = 0; i < child_info->next_invalid_page; i++) {
          copy_virtual_page(parent_info, child_info, i);
        }
    }

    return 0;
}


void vm_switch(pid_t pid) {
    // Check if the given process is valid
    if (process_info_mp.find(pid) == process_info_mp.end()) {
        return; // TODO: More error handling can be done here
    }

    curr_pid = pid;
    page_table_base_register = &(process_info_mp[pid]->page_table)[0];
}


int vm_fault(const void* addr, bool write_flag) {
    shared_ptr<kernel_process_info> process_info = process_info_mp[curr_pid];

    // Get virtual page number of the faulting access
    unsigned int v_page_index = addr_to_index(addr);
    if(v_page_index >= process_info->next_invalid_page || !process_info->page_info_table[v_page_index].valid) return -1;

    auto &faulting_page_info = process_info->page_info_table[v_page_index];
    auto &faulting_pte = process_info_mp[curr_pid]->page_table[v_page_index];

    // Bring data into physical memory if not present
    if (!faulting_page_info.data_location->resident) {
      int success = cache_nonresident_page(faulting_page_info.data_location);
      if(success == -1) return -1;
    }

// DATA IS DEFINITELY IN PHYSICAL MEMORy
    if (write_flag) {
      // Copy on write logic
      if(is_copy_on_writable(faulting_page_info.data_location)) {

        read_fault_handler(faulting_page_info.data_location);

        // Read in old block data to a buffer
        void* old_phys_addr = static_cast<char*>(vm_physmem) + (faulting_page_info.data_location->location * VM_PAGESIZE);
        shared_ptr<mem_block> old_block = faulting_page_info.data_location;
        old_block->ptes.erase(&page_table_base_register[v_page_index]);
        update_block_permissions(old_block);
        memcpy(tmp_buffer.data(), old_phys_addr, VM_PAGESIZE);

        if(free_mem_blocks.empty()) {
            evict_phys_page();
        }
        copy_block_handler(faulting_page_info, &faulting_pte);
      }

      write_fault_handler(faulting_page_info.data_location);
    } // end of if (write_flag)

    read_fault_handler(faulting_page_info.data_location);

    return 0;
}


void vm_destroy() {
    kernel_process_info *process_info = process_info_mp[curr_pid].get();
    unordered_set<unsigned int> useless_mem_blocks;
    for (unsigned int used_virtual_block = 0; used_virtual_block < process_info->next_invalid_page; used_virtual_block++) {

        shared_ptr<mem_block> used_phys_block = process_info->page_info_table[used_virtual_block].data_location;
        used_phys_block->ptes.erase(&page_table_base_register[used_virtual_block]);

        // Do not free blocks that are used for file-backed pages
        if(!used_phys_block->is_swap) {
          if(used_phys_block->ptes.empty() && !used_phys_block->resident) {
            file_mapping_table.erase(std::make_pair(used_phys_block->filename, used_phys_block->block_num));
          }
          continue;
        };

        // Free up the reserved swap blocks held by memory
        free_swap_blocks.push(used_phys_block->swap_blocks.back());
        used_phys_block->swap_blocks.pop_back();

        if(!used_phys_block->resident || used_phys_block == zero_page) continue;

        // If a process exclusively owns a dirty swap backed page, enable it to write
        update_block_permissions(used_phys_block);

        // Free up the physical memory blocks held by only this process
        if(used_phys_block->ptes.empty()) {
          useless_mem_blocks.insert(used_phys_block->location);
          free_mem_blocks.push(used_phys_block->location);
        }
    }

    // Populate the clock queue with only useful memory
    inspect_clock_queue(useless_mem_blocks);

    process_info_mp.erase(curr_pid);
}

void* vm_map(const char* filename, unsigned int block) {

    kernel_process_info *process_info = process_info_mp[curr_pid].get();
    auto &kernel_info_table = process_info->page_info_table;

    // Full arena
    if(process_info->next_invalid_page >= num_virt_addr){ return nullptr; }
    unsigned int new_page = process_info->next_invalid_page;

    /* Swap backed pages */
    if (filename == nullptr) {
        if(free_swap_blocks.empty()) {
            return nullptr;     // eager swap reservation
        }
        map_swap_block(process_info, new_page);
    }

    /* File backed pages */
    else {
        auto [success, filename_str] = read_filename_str(filename);
        if(success == -1) return nullptr;
        auto file_map_key = std::make_pair(filename_str, block);

        // Handling if the file block hasn't been read in
        if(file_mapping_table.find(file_map_key) == file_mapping_table.end()) {
            map_file_block(filename_str, block, new_page);
        } else {
            // File block already exists, point to the data
            shared_ptr<mem_block> used_file_block = file_mapping_table[file_map_key];
            update_file_block(used_file_block, new_page);
        }
    }

    // Newly mapped addresses are always read enabled, but not write enabled
    process_info->next_invalid_page++;
    kernel_info_table[new_page].valid = 1;

    return index_to_addr(new_page);
}
