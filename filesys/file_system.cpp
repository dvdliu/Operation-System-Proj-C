#include "file_system.h"
#include "fs_server.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/mutex.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>

/* LOCK ORDERING (From locking first to last):
1. shared_mutex for a file
2. lock_map_mutex
3. free_block_list_mutex
*/

/* ---------------------- Constructor ---------------------- */

/**
 * @brief Add blocks used by file to used_file_blocks set
 *
 * @param inode_to_search
 * @param used_file_blocks
 */
void bfs_file_inode(fs_inode &inode_to_search,
                    unordered_set<uint32_t> &used_file_blocks) {
  for (uint32_t i = 0; i < inode_to_search.size; i++) {
    uint32_t used_block = inode_to_search.blocks[i];
    used_file_blocks.insert(used_block);
  }
}

/**
 * @brief Update search based on a directory direntries and update used blocks
 *
 * @param inode_to_search
 * @param used_file_blocks
 * @param search
 */
void bfs_directory_inode(fs_inode &inode_to_search,
                         unordered_set<uint32_t> &used_file_blocks,
                         queue<uint32_t> &search) {
  // Account for the blocks used by this inode
  for (uint32_t i = 0; i < inode_to_search.size; i++) {
    uint32_t used_block = inode_to_search.blocks[i];
    used_file_blocks.insert(used_block);

    fs_direntry tmp_block[FS_DIRENTRIES];
    disk_readblock(used_block, tmp_block);

    // Account for the inodes used for each direntry the current inode
    // points to
    for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
      if (tmp_block[j].inode_block != 0) {
        // we are a valid direntry
        search.push(tmp_block[j].inode_block); // Add new inodes to the search
        used_file_blocks.insert(tmp_block[j].inode_block);
      }
    }
  }
}

/**
 * @brief Populate a set of used blocks by the filesystem
 *
 * @param used_file_blocks
 */
void FileSystem::find_free_blocks(unordered_set<uint32_t> &used_file_blocks) {

  // Initialize search with the root inode
  queue<uint32_t> search;
  search.push(0);
  used_file_blocks.insert(0);

  // BFS algorithm
  while (!search.empty()) {
    uint32_t curr_block = search.front();
    search.pop();

    // Read in the most recently searched inode for BFS
    fs_inode inode_to_search;
    disk_readblock(curr_block, &inode_to_search);

    if (inode_to_search.type == 'f') {
      bfs_file_inode(inode_to_search, used_file_blocks);
      continue;
    }

    // inode is type 'd'
    assert(inode_to_search.type == 'd');
    bfs_directory_inode(inode_to_search, used_file_blocks, search);
  }
}

FileSystem::FileSystem() {
  // Initialize free_block_list with blocks that are unused
  unordered_set<uint32_t> used_file_blocks;
  find_free_blocks(used_file_blocks);

  for (unsigned int i = 0; i < FS_DISKSIZE; i++) {
    if (used_file_blocks.find(i) == used_file_blocks.end()) {
      // block i is not in use
      free_block_list.push(i);
    }
  }
}

/* ---------------------- Helper Functions ---------------------- */

/**
 * @brief Get the parent dir pathname
 *
 * @param path
 * @return std::string
 */
std::string get_parent_dir(std::string path) {
  // sanity check
  assert(!path.empty() && path.back() != '/');

  // Find the last occurrence of '/'
  size_t last_slash = path.find_last_of('/');
  assert(last_slash != std::string::npos);

  // Return the path of the parent directory
  return path.substr(0, last_slash);
}

/**
 * @brief Get the target for a path
 *
 * @param path
 * @return std::string
 */
std::string get_target(std::string path) {
  // sanity check
  assert(!path.empty() && path.back() != '/');

  // Find the last occurrence of '/'
  size_t last_slash = path.find_last_of('/');
  assert(last_slash != std::string::npos);

  // Return the targeted filename/dir w/ NO LEADING '/'
  return path.substr(last_slash + 1);
}

/* ---------------------- Private Functions ---------------------- */

void FileSystem::lock_inode_read(uint32_t inode_block) {
  std::shared_ptr<boost::shared_mutex> sm;
  {
    boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

    if (lock_map.find(inode_block) == lock_map.end()) {
      // no lock exists for this file, so we create one
      lock_map[inode_block]; // num_readers = 0
    }

    lock_map[inode_block].num_readers++;
    sm = lock_map[inode_block].m_ptr;
  }
  sm->lock_shared();
}

void FileSystem::unlock_inode_read(uint32_t inode_block) {
  boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

  // Lock must be in the map in order to remove
  assert(lock_map.find(inode_block) != lock_map.end());

  lock_map[inode_block].m_ptr->unlock_shared();
  lock_map[inode_block].num_readers--;

  // Remove lock from map if this is the last reader/writer using this lock
  if (lock_map[inode_block].num_readers == 0) {
    lock_map.erase(inode_block);
  }
}

void FileSystem::lock_inode_upgrade(uint32_t inode_block) {
  std::shared_ptr<boost::shared_mutex> sm;
  {
    boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

    if (lock_map.find(inode_block) == lock_map.end()) {
      // no lock exists for this file, so we create one
      lock_map[inode_block]; // num_readers = 0
    }

    lock_map[inode_block].num_readers++;
    sm = lock_map[inode_block].m_ptr;
  }
  sm->lock_upgrade();
}

void FileSystem::unlock_inode_upgrade(uint32_t inode_block) {
  boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

  // Lock must be in the map in order to remove
  assert(lock_map.find(inode_block) != lock_map.end());

  lock_map[inode_block].m_ptr->unlock_upgrade();
  lock_map[inode_block].num_readers--;

  // Remove lock from map if this is the last reader/writer using this lock
  if (lock_map[inode_block].num_readers == 0) {
    lock_map.erase(inode_block);
  }
}

void FileSystem::upgrade_to_write_lock(uint32_t inode_block) {
  std::shared_ptr<boost::shared_mutex> sm;
  {
    boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

    // lock must be in map to make writer
    assert(lock_map.find(inode_block) != lock_map.end());

    // no need to update anything as we consider a writer to be a reader
    sm = lock_map[inode_block].m_ptr;
  }
  sm->unlock_upgrade_and_lock();
}

void FileSystem::unlock_inode_write(uint32_t inode_block) {
  boost::lock_guard<boost::mutex> lock(this->lock_map_mutex);

  // lock must be in map to unlock
  assert(lock_map.find(inode_block) != lock_map.end());

  lock_map[inode_block].m_ptr->unlock();
  // just decrement readers, since we are a writer (therefore a reader)
  // and we stop holding lock
  lock_map[inode_block].num_readers--;

  // Remove lock from map if this is the last reader/writer using this lock
  if (lock_map[inode_block].num_readers == 0) {
    lock_map.erase(inode_block);
  }
}

uint32_t FileSystem::find_parent_inode(const string &path_name,
                                       const string &user_name) {

  // Get the parent
  std::string parent_path = get_parent_dir(path_name);
  std::deque<std::string> directory_names;
  std::istringstream stream(parent_path);
  std::string dir_name;

  // populates directory_names with the names from the pathname
  while (std::getline(stream, dir_name, '/')) {
    directory_names.push_back(dir_name);
  }

  // Keep track of the block storing the next inode
  uint32_t target_inode_block = ROOT_INODE;
  uint32_t prev_inode_block = UNDEFINED_BLOCK;
  boost::shared_mutex prev_lock;

  lock_inode_read(target_inode_block);

  while (directory_names.size() > 1) {
    // Get the directory name to look for
    std::string dir_name = directory_names.front();
    if (strcmp(dir_name.c_str(), "") == 0) {
      assert(target_inode_block == ROOT_INODE);
    }
    directory_names.pop_front();

    // Set the curr inode to the prev, and get the next inode block
    prev_inode_block = target_inode_block;
    target_inode_block = this->find_next_inode(directory_names.front(),
                                               target_inode_block, user_name);

    // Hand over hand locking
    lock_inode_read(target_inode_block);
    unlock_inode_read(prev_inode_block);
  }

  // Returns the inode for the last directory in the path_name
  return target_inode_block; // Target inode block will have a lock_shared
                             // shared mutex!
}

uint32_t FileSystem::find_next_inode(const string &next_dir_name,
                                     uint32_t target_inode_block,
                                     const string &user_name) {

  // !! WE HAVE A READER LOCK ON TARGET_INODE_BLOCK FOR THIS WHOLE FUNCTION

  // Read in the inode for the file we are looking for (should be found by prev
  // iteration)
  fs_inode curr_inode;
  disk_readblock(target_inode_block, &curr_inode);

  // Check for owner permissions if not at the root directory
  if (target_inode_block != 0 && user_name != curr_inode.owner) {
    unlock_inode_read(target_inode_block);
    throw std::runtime_error("User is not owner of this dir");
  }

  if (curr_inode.type != 'd') {
    unlock_inode_read(target_inode_block);
    throw std::runtime_error("Expected a directory, but got a file");
  }

  uint32_t result = UNDEFINED_BLOCK;

  // Look for the current inode direntries to find the next inode
  for (unsigned int i = 0; i < curr_inode.size; i++) {
    fs_direntry tmp_block[FS_DIRENTRIES];
    disk_readblock(curr_inode.blocks[i], tmp_block);

    for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
      if (tmp_block[j].inode_block == 0)
        continue; // skip unused direntries
      if (strcmp(tmp_block[j].name, next_dir_name.c_str()) == 0) {
        result = tmp_block[j].inode_block;
        break;
      }
    }

    if (result != UNDEFINED_BLOCK) {
      // we found the dir_entry, no need to continue looping
      break;
    }
  }

  if (result == UNDEFINED_BLOCK) {
    unlock_inode_read(target_inode_block);
    throw std::runtime_error("Directory not found");
  }

  return result;
}

/* ---------------------- Public Functions ---------------------- */

/* ---------------------- READ ---------------------- */
array<byte, FS_BLOCKSIZE> FileSystem::handle_read(const Command &command) {

  /* ------ READING FILE INODE, CHECKING VALIDITY ------ */
  int parent_inode_block =
      this->find_parent_inode(command.pathname, command.username);
  array<byte, FS_BLOCKSIZE> data_read;

  // !! WE CURRENTLY HAVE A READER LOCK FOR PARENT_INODE_BLOCK

  // Get the target filename/direntry and its inode
  string target = get_target(command.pathname);
  int target_inode_block =
      find_next_inode(target, parent_inode_block, command.username);

  // Hand over hand locking
  lock_inode_read(target_inode_block);
  unlock_inode_read(parent_inode_block);

  // !! WE CURRENTLY HAVE A READER LOCK FOR THE TARGET_INODE_BLOCK

  fs_inode target_inode;
  disk_readblock(target_inode_block, &target_inode);

  // Error checking: ensure file read is a file, and that the file block is
  // within the file size
  if (target_inode.type != 'f' || command.block >= target_inode.size) {
    unlock_inode_read(target_inode_block);
    throw std::runtime_error("Provided file is not a valid file");
  }

  if (command.username != target_inode.owner) {
    unlock_inode_read(target_inode_block);
    throw std::runtime_error("User is not owner of file");
  }

  /* ------ VERIFIED CAN READ FROM FILE, READING ------ */

  // Read the requested data into the command struct
  disk_readblock(target_inode.blocks[command.block], data_read.data());

  unlock_inode_read(target_inode_block);

  // !! WE NOW HAVE UNLOCKED ALL LOCKS

  return data_read;
}

/* ---------------------- WRITE ---------------------- */
void FileSystem::handle_write(const Command &command) {

  /* ------ READING FILE INODE, CHECKING VALIDITY ------ */
  int parent_inode_block =
      this->find_parent_inode(command.pathname, command.username);

  // !! WE CURRENTLY HAVE A READER LOCK FOR PARENT_INODE_BLOCK

  // Get the target filename/direntry and its inode
  string target = get_target(command.pathname);
  int target_inode_block =
      find_next_inode(target, parent_inode_block, command.username);

  // Hand over hand locking: get upgradeable lock, give up read lock
  lock_inode_upgrade(target_inode_block);
  unlock_inode_read(parent_inode_block);

  // !! WE CURRENTLY HAVE A UPGRADEABLE LOCK FOR TARGET_INODE_BLOCK

  fs_inode target_inode;
  disk_readblock(target_inode_block, &target_inode);

  // Error checking: ensure file written to is a file, and that the file block
  // is within the file size
  if (target_inode.type != 'f' || command.block > target_inode.size) {
    unlock_inode_upgrade(target_inode_block);
    throw std::runtime_error("Provided block is not writable");
  }

  if (command.username != target_inode.owner) {
    unlock_inode_upgrade(target_inode_block);
    throw std::runtime_error("User is not owner of file");
  }

  /* ------ VERIFIED CAN WRITE TO FILE, WRITING ------ */

  bool increase_file_size = command.block == target_inode.size;
  if (increase_file_size) {
    // we are writing to a new block (the next consecutive block)
    // we need to adjust our inode metadata accordingly

    boost::lock_guard<boost::mutex> free_block_list_lock(free_block_list_mutex);

    if (free_block_list.empty() || target_inode.size == FS_MAXFILEBLOCKS) {
      unlock_inode_upgrade(target_inode_block);
      throw std::runtime_error("Not enough disk space for this write");
    }

    uint32_t free_block = free_block_list.front();
    try {
      free_block_list.pop();

      target_inode.blocks[target_inode.size] = free_block;
      target_inode.size++;
    } catch (std::runtime_error &e) {
      free_block_list.push(free_block);
      throw;
    }
  }

  if (!increase_file_size) {
    // we verified we can write, now we grab a writer lock
    upgrade_to_write_lock(target_inode_block);

    // !! WE CURRENTLY HAVE A WRITER LOCK FOR TARGET_INODE_BLOCK

    // write to the specified block of the file
    disk_writeblock(target_inode.blocks[command.block], command.data);
  } else {
    // write to the specified block of the file
    disk_writeblock(target_inode.blocks[command.block], command.data);

    // we verified we can write, now we grab a writer lock
    upgrade_to_write_lock(target_inode_block);

    // !! WE CURRENTLY HAVE A WRITER LOCK FOR TARGET_INODE_BLOCK

    disk_writeblock(target_inode_block, &target_inode);
  }

  unlock_inode_write(target_inode_block);

  // !! WE NOW HAVE UNLOCKED ALL LOCKS
}

/* ---------------------- CREATE ---------------------- */

void FileSystem::handle_create(const Command &command) {

  std::string parent_path = get_parent_dir(command.pathname);
  std::string target_name = get_target(command.pathname);

  uint32_t parent_inode_block =
      get_parent_inode_and_upgradeable_lock(command, parent_path);

  // !! WE CURRENTLY HAVE AN UPGRADEABLE LOCK FOR PARENT_INODE_BLOCK

  fs_inode parent_inode;
  disk_readblock(parent_inode_block, &parent_inode);
  validate_parent_inode(command, parent_path, parent_inode, parent_inode_block);

  // Check parent inode data for unused direntries that we can use
  bool is_empty_direntry = false;
  uint32_t new_direntry_block = UNDEFINED_BLOCK;
  uint32_t new_direntry_location = 0;
  fs_direntry updated_direntries_block[FS_DIRENTRIES];

  memset(updated_direntries_block, 0, FS_BLOCKSIZE);

  for (uint32_t i = 0; i < parent_inode.size; i++) {
    fs_direntry tmp_block[FS_DIRENTRIES];
    disk_readblock(parent_inode.blocks[i], tmp_block);

    // Look for free space and check for duplicate names
    for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
      if (tmp_block[j].inode_block == 0) { // FREE SPACE
        if (!is_empty_direntry) {          // pick the lowest direntry space
          is_empty_direntry = true;
          new_direntry_location = j;
          new_direntry_block = parent_inode.blocks[i];

          std::memcpy(updated_direntries_block, tmp_block, sizeof(tmp_block));
        }
      } else { // DUPLICATE NAME
        if (strcmp(tmp_block[j].name, target_name.c_str()) == 0) {
          unlock_inode_upgrade(parent_inode_block);
          throw std::runtime_error("Path already exists");
        }
      }
    }
  }

  // sanity check, ensure that direntry block to write is invalid if no empty
  // space is found
  if (!is_empty_direntry) {
    for (unsigned int i = 0; i < FS_DIRENTRIES; i++) {
      updated_direntries_block[i].inode_block = 0;
    }
  }

  uint32_t new_inode_block = UNDEFINED_BLOCK;
  {
    boost::lock_guard<boost::mutex> lock(free_block_list_mutex);
    if (free_block_list.empty()) {
      throw std::runtime_error("Not enough disk space for create");
    }
    new_inode_block = free_block_list.front();
    free_block_list.pop();
  }

  updated_direntries_block[new_direntry_location].inode_block = new_inode_block;
  strcpy(updated_direntries_block[new_direntry_location].name,
         target_name.c_str());

  // initialize new file/directory to write to disk
  fs_inode new_inode;
  new_inode.size = 0;
  strcpy(new_inode.owner, command.username.c_str());
  new_inode.type = command.type;

  if (!is_empty_direntry) {
    // NO EMPTY DIR BLOCK, NEED TO GRAB FREE BLOCK
    {
      boost::lock_guard<boost::mutex> free_blocks_lock(free_block_list_mutex);
      if (free_block_list.empty() || parent_inode.size == FS_MAXFILEBLOCKS) {
        unlock_inode_upgrade(parent_inode_block);
        throw std::runtime_error("Not enough disk space for create");
      }

      new_direntry_block = free_block_list.front();
      parent_inode.blocks[parent_inode.size] = new_direntry_block;
      free_block_list.pop();
    }
  }

  disk_writeblock(new_inode_block, &new_inode);

  if (is_empty_direntry) {
    upgrade_to_write_lock(parent_inode_block);

    // !! WE CURRENTLY HAVE A WRITER LOCK FOR PARENT_INODE_BLOCK

    disk_writeblock(new_direntry_block, updated_direntries_block);
  } else {

    parent_inode.size++;

    disk_writeblock(new_direntry_block, updated_direntries_block);

    upgrade_to_write_lock(parent_inode_block);

    // !! WE CURRENTLY HAVE A WRITER LOCK FOR PARENT_INODE_BLOCK

    disk_writeblock(parent_inode_block, &parent_inode);
  }

  unlock_inode_write(parent_inode_block);

  // !! WE NOW HAVE UNLOCKED ALL LOCKS
}

/* ---------------------- DELETE ---------------------- */

uint32_t
FileSystem::get_parent_inode_and_upgradeable_lock(const Command &command,
                                                  std::string parent_path) {
  // ! we should not be holding any locks when calling this function

  uint32_t parent_inode_block;
  if (parent_path.empty()) {
    // Parent is root
    parent_inode_block = ROOT_INODE;
    lock_inode_upgrade(parent_inode_block);
  } else {
    // Find parent via find_parent_inode
    uint32_t grandparent_inode_block =
        this->find_parent_inode(parent_path, command.username);

    // !! WE CURRENTLY HAVE A READER LOCK FOR GRANDPARENT_INODE_BLOCK

    // Get parent directory name and find parent inode block
    std::string parent_dirname = get_target(parent_path);
    parent_inode_block = find_next_inode(
        parent_dirname, grandparent_inode_block, command.username);

    // Hand-over-hand locking
    lock_inode_upgrade(parent_inode_block);
    unlock_inode_read(grandparent_inode_block);
  }

  return parent_inode_block;
}

void FileSystem::validate_parent_inode(const Command &command,
                                       const std::string &parent_path,
                                       const fs_inode &parent_inode,
                                       uint32_t parent_inode_block) {
  // ! we should be holding an upgrade lock on parent_inode_block when here

  if (!parent_path.empty() &&
      (parent_inode.type != 'd' || parent_inode.owner != command.username)) {
    unlock_inode_upgrade(parent_inode_block);
    throw std::runtime_error(
        "Expected a directory, but got a file or user is not owner lol");
  }
}

bool FileSystem::is_block_empty(fs_direntry *dir_block) {
  for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
    if (dir_block[j].inode_block != 0) {
      return false;
    }
  }
  return true;
}

void FileSystem::handle_delete(const Command &command) {
  // ----- Step 1: Parse the path to get parent and target -----
  if (strcmp(command.pathname.c_str(), "/") == 0) {
    throw std::runtime_error("Attempting to delete root");
  }

  std::string parent_path = get_parent_dir(command.pathname);
  std::string target_name = get_target(command.pathname);

  // ----- Step 2: Find the parent inode block and lock it (upgrade lock) -----
  uint32_t parent_inode_block =
      get_parent_inode_and_upgradeable_lock(command, parent_path);

  // !! WE CURRENTLY HAVE AN UPGRADEABLE LOCK FOR PARENT_INODE_BLOCK

  fs_inode parent_inode;
  disk_readblock(parent_inode_block, &parent_inode);
  validate_parent_inode(command, parent_path, parent_inode, parent_inode_block);

  // ----- Step 3: Find the child's inode block -----
  bool found = false;
  uint32_t child_inode_block = UNDEFINED_BLOCK;
  uint32_t dir_block_index =
      UNDEFINED_BLOCK; // which parent_inode.blocks[i] contains the entry
  uint32_t dir_entry_index = UNDEFINED_BLOCK; // which entry in that block
  fs_direntry dir_block[FS_DIRENTRIES];

  for (unsigned int i = 0; i < parent_inode.size && !found; ++i) {
    disk_readblock(parent_inode.blocks[i], dir_block);
    for (unsigned int j = 0; j < FS_DIRENTRIES; ++j) {
      if (dir_block[j].inode_block != 0 &&
          strcmp(dir_block[j].name, target_name.c_str()) == 0) {
        child_inode_block = dir_block[j].inode_block;
        dir_block_index = i;
        dir_entry_index = j;
        found = true;
        break;
      }
    }
  }

  if (!found) {
    // No such file/directory
    unlock_inode_upgrade(parent_inode_block);
    throw std::runtime_error("Target does not exist");
  }

  // ----- Step 4: Lock the child inode for inspection -----
  // ! GRAB PARENT WRITER LOCK SO THAT WE DONT DEADLOCK
  upgrade_to_write_lock(parent_inode_block);
  lock_inode_upgrade(child_inode_block);

  // !! WE CURRENTLY HAVE AN WRITER LOCK FOR PARENT_INODE_BLOCK
  // !! WE CURRENTLY HAVE AN UPGRADEABLE LOCK FOR CHILD_INODE_BLOCK

  fs_inode child_inode;
  disk_readblock(child_inode_block, &child_inode);

  // checking if ownership is valid
  if (child_inode.owner != command.username) {
    unlock_inode_write(parent_inode_block);
    unlock_inode_upgrade(child_inode_block);
    throw std::runtime_error("User is not owner of the target");
  }

  // child_inode is a directory
  if (child_inode.type == 'd' && child_inode.size != 0) {
    unlock_inode_write(parent_inode_block);
    unlock_inode_upgrade(child_inode_block);
    throw std::runtime_error("Directory not empty");
  }

  // clear the entry
  dir_block[dir_entry_index].inode_block = 0; // mark unused
  memset(dir_block[dir_entry_index].name, 0, FS_MAXFILENAME + 1);

  // check if the block is empty
  if (is_block_empty(dir_block)) {
    uint32_t empty_block = parent_inode.blocks[dir_block_index];

    // shift all the subsequent dir_blocks left
    for (unsigned int i = dir_block_index; i < parent_inode.size - 1; ++i) {
      parent_inode.blocks[i] = parent_inode.blocks[i + 1];
    }
    parent_inode.size--;

    disk_writeblock(parent_inode_block, &parent_inode);

    {
      // free the original empty block
      boost::lock_guard<boost::mutex> lock(this->free_block_list_mutex);
      free_block_list.push(empty_block);
    }
  } else {
    disk_writeblock(parent_inode.blocks[dir_block_index], dir_block);
  }

  // !! WE CURRENTLY HAVE AN UPGRADEABLE LOCK FOR CHILD_INODE_BLOCK

  upgrade_to_write_lock(child_inode_block);
  unlock_inode_write(parent_inode_block);

  // !! WE CURRENTLY HAVE AN WRITER LOCK FOR CHILD_INODE_BLOCK

  // free the child's inode block
  {
    boost::lock_guard<boost::mutex> lock(this->free_block_list_mutex);
    free_block_list.push(child_inode_block);
  }

  // free the child's inode's file blocks
  if (child_inode.type == 'f') {
    // free all the file blocks back
    for (unsigned int i = 0; i < child_inode.size; ++i) {
      uint32_t data_block = child_inode.blocks[i];
      // Return data_block to free list
      boost::lock_guard<boost::mutex> lock(this->free_block_list_mutex);
      free_block_list.push(data_block);
    }
  }

  unlock_inode_write(child_inode_block);

  // !! WE HAVE NOW UNLOCKED ALL LOCKS
}