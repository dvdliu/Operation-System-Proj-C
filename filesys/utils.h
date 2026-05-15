#include "fs_server.h"
#include <cstddef>
#include <memory>

enum class CommandType {
  FS_READBLOCK,
  FS_WRITEBLOCK,
  FS_CREATE,
  FS_DELETE,
  INVALID
};

static constexpr uint32_t ROOT_INODE = 0;

/**
 * @brief Hold request data from a client to pass to the file system
 *
 */
struct Command {
  CommandType command_type;
  std::string username;
  std::string pathname;
  u_int32_t block;              // only used for read/writes
  char type;                    // only used for creates
  std::byte data[FS_BLOCKSIZE]; // only used for writes
};

/**
 * @brief Hold information stored in a file_system lock_map
 *
 */
struct GyattLock {
  std::shared_ptr<boost::shared_mutex> m_ptr;

  unsigned int num_readers;

  inline GyattLock()
      : m_ptr(std::make_shared<boost::shared_mutex>()), num_readers(0) {}
};