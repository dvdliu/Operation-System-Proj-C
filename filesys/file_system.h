
#pragma once

#include "fs_server.h"
#include "utils.h"
#include <array>
#include <cstdint>
#include <cstddef>
#include <deque>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

using std::array;
using std::deque;
using std::queue;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::byte;

static constexpr uint32_t UNDEFINED_BLOCK = FS_DISKSIZE + 1;

class FileSystem {
public:
  FileSystem();

  /**
   * @brief Read a block in a file from the file system
   * 
   * @param command 
   * @return array<char, FS_BLOCKSIZE> 
   */
  array<byte, FS_BLOCKSIZE> handle_read(const Command &command);

  /**
   * @brief Write to a block in a file in the file system
   * 
   * @param command 
   */
  void handle_write(const Command &command);

  /**
   * @brief Create a file/directory in the filesystem
   * 
   * @param command 
   */
  void handle_create(const Command &command);

  /**
   * @brief Delete a file/empty directory in the filesystem
   * 
   * @param command 
   */
  void handle_delete(const Command &command);

  queue<uint32_t> free_block_list;              // unused disk blocks
  boost::mutex free_block_list_mutex;           // mutex for unused disk block list
  unordered_map<uint32_t, GyattLock> lock_map;  // map of locks per file/directory
  boost::mutex lock_map_mutex;                  // mutex for lock_map

private:
  /**
   * @brief Populate free_block_list with unused blocks in the filesystem
   * 
   * @param used_file_blocks 
   */
  void find_free_blocks(unordered_set<uint32_t> &used_file_blocks);

  uint32_t find_parent_inode(const string &path_name, const string &user_name);
  /**
   * @brief Searches for the next inode corresponding to `next_dir_name`
   * accessible by `user_name`.
   *
   * This function reads the inode at `target_inode_block` and searches its
   * directory entries to find an entry matching `next_dir_name`. It checks if
   * `user_name` has access permissions.
   *
   * @param next_dir_name The name of the next directory or file to find.
   * @param target_inode_block The block number of the current inode to search
   * in.
   * @param user_name The name of the user requesting access.
   * @return The inode block number if found and accessible; returns
   * UNDEFINED_BLOCK if not found or access denied.
   */
  uint32_t find_next_inode(const string &next_dir_name,
                           uint32_t target_inode_block,
                           const string &user_name);

  /**
   * The following functions are utility functions that are used to update the lock_map
   * based on whether a reader/writer lock is acquired.
   */
  void lock_inode_read(uint32_t inode_block);
  void unlock_inode_read(uint32_t inode_block);

  void lock_inode_upgrade(uint32_t inode_block);
  void unlock_inode_upgrade(uint32_t inode_block);

  void upgrade_to_write_lock(uint32_t inode_block);
  void unlock_inode_write(uint32_t inode_block);

  /* Helper functions for fs_delete and fs_create */
  /**
   * @brief Get the parent inode block number and its corresponding upgraded lock to write to
   *  - Used by fs_create and fs_delete when we are modifying the inode of the parent directory
   * 
   * @param command 
   * @param parent_path 
   * @return uint32_t 
   */
  uint32_t get_parent_inode_and_upgradeable_lock(const Command &command,
                                                 std::string parent_path);
  /**
   * @brief Validate a
   * 
   * @param command 
   * @param parent_path 
   * @param parent_inode 
   * @param parent_inode_block 
   */
  void validate_parent_inode(const Command &command,
                             const std::string &parent_path,
                             const fs_inode &parent_inode,
                             uint32_t parent_inode_block);
  /**
   * @brief Check if a direntry block is empty
   * - used after deletion in fs_delete on parent inode
   * 
   * @param dir_block 
   * @return true 
   * @return false 
   */
  bool is_block_empty(fs_direntry *dir_block);
};
