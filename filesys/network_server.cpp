#include "network_server.h"
#include "file_system.h"
#include "fs_param.h"
#include "fs_server.h"
#include "socket_raii.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// setup filesystem first as per 5.2, then
// initializes socket and starts opens to listening
NetworkServer::NetworkServer(int input_port) : my_filesystem() {

  server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd == -1) {
    perror("Error opening stream socket");
    throw std::runtime_error("Error opening stream socket");
  }

  int yes_opt_val = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes_opt_val,
                 sizeof(yes_opt_val)) == -1) {
    perror("Error setting socket options");
    throw std::runtime_error("Error setting socket options");
  }

  sockaddr_in addr{}; // initializes sockaddr_in struct with all 0s
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(input_port);

  if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) ==
      -1) {
    perror("Error binding stream socket");
    throw std::runtime_error("Error binding stream socket");
  }

  if (listen(server_fd, 30) == -1) {
    perror("Error starting to listen");
    throw std::runtime_error("Error starting to listen");
  }

  port = get_port_number(server_fd);
  if (port == -1) { // bro this feels like golang
    throw std::runtime_error("Error getting socket port number");
  }

  print_port(port);
}

// main logic of server, running and accepting connections opening a new thread
// every time one is formed
void NetworkServer::run() {
  while (true) {
    int client_socket = accept(
        server_fd, NULL,
        NULL); // assuming we do not need to keep track of client network data
    if (client_socket == -1) {
      perror("Error accepting a connection");
      // we just ignore these errors and go next
      continue;
    }

    boost::thread t(&NetworkServer::handle_request, this, client_socket);
    t.detach(); // automatically freeing up resources after thread is done (?)
  }
}

// handler for each request
// should be called with a new thread -> top of stack
int NetworkServer::handle_request(int client_socket) {
  SocketRAII my_socket(client_socket); // handles closing socket in all cases

  std::string request_str;
  if (receive_message(client_socket, request_str) == -1) {
    // Handle error if necessary
    return -1;
  }

  std::istringstream iss(request_str);
  std::string command_str;
  iss >> command_str;

  CommandType command = parse_command(command_str);
  int return_val;

  switch (command) {
  case CommandType::FS_READBLOCK:
    return_val = handle_read(client_socket, request_str);
    break;

  case CommandType::FS_WRITEBLOCK:
    return_val = handle_write(client_socket, request_str);
    break;

  case CommandType::FS_CREATE:
    return_val = handle_create(client_socket, request_str);
    break;

  case CommandType::FS_DELETE:
    return_val = handle_delete(client_socket, request_str);
    break;

  default:
    return_val = -1;
    break;
  }

  if (return_val == -1) {
    // error parsing response, we will just close the connection
    return -1;
  }

  return 0;
}

// ------------------------ PRIVATE CLASS FUNCTIONS ------------------------

// gets the current port number from the socket
int NetworkServer::get_port_number(int sockfd) {
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1) {
    perror("Error getting socket port number");
    return -1;
  }

  return ntohs(sin.sin_port);
}

// receives message to completion or to error
int NetworkServer::receive_message(int sockfd, std::string &request_str) {
  assert(request_str == "");
  char c;
  ssize_t rval;

  while (true) {
    rval = recv(sockfd, &c, 1, 0);
    if (rval == -1) {
      perror("Error reading stream message");
      return -1;
    } else if (rval == 0) {
      // client closed connection
      return -1;
    }

    // if null terminator -> stop reading
    if (c == '\0') {
      break;
    }

    request_str.push_back(c);

    // check if the message size exceeds MAX_MESSAGE_LEN
    if (request_str.size() > MAX_MESSAGE_SIZE) {
      perror("Error: Message exceeds maximum length\n");
      return -1;
    }
  }

  return 0;
}

int NetworkServer::send_message(int sockfd, const std::string &response_str) {
  size_t message_len = response_str.size() + 1;
  const char *msg = response_str.c_str();

  size_t sent = 0;
  do {
    const ssize_t n =
        send(sockfd, msg + sent, message_len - sent, MSG_NOSIGNAL);
    if (n == -1) {
      perror("Error sending on stream socket");
      return -1;
    }
    sent += n;
  } while (sent < message_len);

  return 0;
}

CommandType NetworkServer::parse_command(const std::string &command_str) {
  // can't use switch statements for strings ig
  if (command_str == "FS_READBLOCK") {
    return CommandType::FS_READBLOCK;
  } else if (command_str == "FS_WRITEBLOCK") {
    return CommandType::FS_WRITEBLOCK;
  } else if (command_str == "FS_CREATE") {
    return CommandType::FS_CREATE;
  } else if (command_str == "FS_DELETE") {
    return CommandType::FS_DELETE;
  } else {
    return CommandType::INVALID;
  }
}

Command NetworkServer::parse_message(const std::string &request_str) {
  std::istringstream iss(request_str);
  std::string command_str;
  Command command;
  iss >> command_str >> command.username >> command.pathname;

  // sanity check -> this should never happen unless my parse_command is
  // bugging
  assert(!command_str.empty());

  if (command.username.empty()) {
    throw std::runtime_error("Error: Username is empty\n");
  }
  if (command.username.size() > FS_MAXUSERNAME) {
    throw std::runtime_error("Error: Username too long\n");
  }

  // pathname validation
  if (command.pathname.empty()) {
    throw std::runtime_error("Error: Pathname is empty\n");
  }
  if (!is_valid_pathname(command.pathname)) {
    throw std::runtime_error("Error: Pathname is invalid\n");
  }

  command.command_type = parse_command(command_str);

  // handles block number parsing for FS_READ/WRITEBLOCK
  if (command.command_type == CommandType::FS_READBLOCK ||
      command.command_type == CommandType::FS_WRITEBLOCK) {
    std::string block_str;

    iss >> block_str;
    if (block_str.empty()) {
      throw std::runtime_error("Error: Block number is empty\n");
    }
    if (block_str.size() > 1 && block_str[0] == '0') {
      throw std::runtime_error("Error: Block number has a leading zero\n");
    }

    // should throw runtime error if not properly convertible
    command.block = std::stoul(block_str);

    if (command.block >= FS_MAXFILEBLOCKS) {
      throw std::runtime_error("Error: Block number exceeds FS_MAXFILEBLOCKS");
    }
  }

  // handles type parsing for FS_CREATE
  if (command.command_type == CommandType::FS_CREATE) {
    iss >> command.type;
    if (command.type != 'f' && command.type != 'd') {
      throw std::runtime_error("Error: Command type is not 'f' or 'd'\n");
    }
  }

  std::string temp;
  if (iss >> temp) { // while?
    throw std::runtime_error("Error: Request contains too many arguments\n");
  }

  return command;
}

bool NetworkServer::is_valid_pathname(std::string pathname) {
  assert(!pathname.empty());

  if (pathname.size() > FS_MAXPATHNAME) {
    return false;
  }

  if (pathname[0] != '/') {
    // pathname does not start with '/'
    return false;
  }

  // removing check for root
  // if (pathname.size() == 1) {
  //   // pathname is root
  //   return true;
  // }

  if (pathname[pathname.size() - 1] == '/') {
    // pathname ends with a '/' and it is not the root dir ('/')
    return false;
  }

  // checking for // or whitespace
  for (size_t i = 0; i < pathname.size() - 1; ++i) {
    if (pathname[i] == '/' && pathname[i + 1] == '/') {
      // two '/' in a row
      return false;
    }
    if (std::isspace(static_cast<unsigned char>(pathname[i]))) {
      // there is whitespace present
      return false;
    }
  }

  // checking for max_filename length
  std::string path_sub = pathname.substr(1);
  std::istringstream path_iss(path_sub);
  std::string component;
  while (std::getline(path_iss, component, '/')) {
    if (component.empty()) {
      // should have been caught by previous check for //
      return false;
    }
    if (component.size() > FS_MAXFILENAME) {
      return false;
    }
  }

  return true;
}

int NetworkServer::handle_read(int sockfd, const std::string &request_str) {
  try {
    Command command = parse_message(request_str);
    array<byte, FS_BLOCKSIZE> data = my_filesystem.handle_read(command);

    // first, send back the original message
    if (send_message(sockfd, request_str) == -1) {
      perror("Error sending handle_read original message back");
      return -1;
    }

    // now, send the FS_BLOCKSIZE bytes of data
    ssize_t sent = 0;
    const byte *block_data = data.data();
    while (sent < FS_BLOCKSIZE) {
      ssize_t n =
          send(sockfd, block_data + sent, FS_BLOCKSIZE - sent, MSG_NOSIGNAL);
      if (n == -1) {
        perror("Error sending file data");
        return -1;
      }
      sent += n;
    }
  } catch (...) {
    return -1;
  }

  return 0;
}

int NetworkServer::handle_write(int sockfd, const std::string &request_str) {
  try {
    Command command = parse_message(request_str);

    // recv the next block_size bytes of data
    ssize_t received = 0;
    while (received < FS_BLOCKSIZE) {
      ssize_t rval =
          recv(sockfd, command.data + received, FS_BLOCKSIZE - received, 0);
      if (rval == -1) {
        perror("Error reading block data");
        return -1;
      } else if (rval == 0) {
        // client closed connection
        // !! maybe double check logic here -> i think just returning is ok
        return -1;
      }
      received += rval;
    }

    // TODO call File Server function
    my_filesystem.handle_write(command);

    if (send_message(sockfd, request_str) == -1) {
      perror("Error sending handle_write original message back");
      return -1;
    }

  } catch (...) {
    return -1;
  }

  return 0;
}

int NetworkServer::handle_create(int sockfd, const std::string &request_str) {
  try {
    Command command = parse_message(request_str);

    // TODO call File Server function
    my_filesystem.handle_create(command);

    if (send_message(sockfd, request_str) == -1) {
      perror("Error sending handle_create original message back");
      return -1;
    }
  } catch (...) {
    return -1;
  }

  return 0;
}

int NetworkServer::handle_delete(int sockfd, const std::string &request_str) {
  try {
    Command command = parse_message(request_str);

    // TODO call File Server function
    my_filesystem.handle_delete(command);

    if (send_message(sockfd, request_str) == -1) {
      perror("Error sending handle_delete original message back");
      return -1;
    }
  } catch (...) {
    return -1;
  }

  return 0;
}