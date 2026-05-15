/*
 * network_server.h
 *
 * Header file for the network server.
 */

#pragma once

#include "file_system.h"
#include "fs_param.h"
#include "fs_server.h"
#include <string>
#include <sys/types.h>

// 13 (max command len) + max username + max pathname + 3 (max block len) + 3
// (spaces) + 1 (nullterm)
static constexpr unsigned int MAX_MESSAGE_SIZE =
    FS_MAXUSERNAME + FS_MAXPATHNAME + 20;

class NetworkServer {
public:
  /**
   * @brief Constructor for the NetworkServer class.
   * Sets up the file system, initializes the server socket, and starts
   * listening for connections.
   * @param input_port Port number to bind the server socket to. If set to 0,
   * the OS selects an available port.
   * @throws std::runtime_error if socket creation, binding, or listening fails.
   */
  NetworkServer(int port);

  /**
   * @brief Main server logic to accept and handle incoming client connections.
   *        Opens a new thread for each client request.
   */
  void run();

private:
  /**
   * @brief Handles a single client request. Determines the command type and
   * dispatches it to the appropriate handler.
   * @param client_socket Socket file descriptor for the client connection.
   * @return 0 on success, -1 on error.
   */
  int handle_request(int client_socket);

  /**
   * @brief Retrieves the port number associated with the given socket file
   * descriptor.
   * @param sockfd Socket file descriptor.
   * @return The port number in host byte order, or -1 if an error occurs.
   */
  int get_port_number(int sockfd);

  /**
   * @brief Receives a null-terminated message from a socket.
   * @param sockfd Socket file descriptor.
   * @param request_str String to store the received message.
   * @return 0 on success, -1 on error.
   */
  int receive_message(int sockfd, std::string &request_str);

  /**
   * @brief Sends a null-terminated message to a socket.
   * @param sockfd Socket file descriptor.
   * @param response_str Message to send.
   * @return 0 on success, -1 on error.
   */
  int send_message(int sockfd, const std::string &response_str);

  /**
   * @brief Parses the command string from a client request.
   * @param command_str String representation of the command.
   * @return The corresponding CommandType enum value.
   */
  CommandType parse_command(const std::string &command_str);

  /**
   * @brief Parses a full client request string into a Command structure.
   * @param request_str Full request string from the client.
   * @return A Command structure containing parsed details of the request.
   * @throws std::runtime_error if parsing fails or validation errors occur.
   */
  Command parse_message(const std::string &request_str);

  /**
   * @brief Validates a file or directory pathname.
   * @param pathname Pathname string to validate.
   * @return true if the pathname is valid, false otherwise.
   */
  bool is_valid_pathname(std::string pathname);

  /**
   * @brief Handles an FS_READBLOCK request from a client.
   * @param sockfd Socket file descriptor for the client connection.
   * @param request_str Full request string from the client.
   * @return 0 on success, -1 on error.
   */
  int handle_read(int sockfd, const std::string &request_str);
  /**
   * @brief Handles an FS_WRITEBLOCK request from a client.
   * @param sockfd Socket file descriptor for the client connection.
   * @param request_str Full request string from the client.
   * @return 0 on success, -1 on error.
   */
  int handle_write(int sockfd, const std::string &request_str);
  /**
   * @brief Handles an FS_CREATE request from a client.
   * @param sockfd Socket file descriptor for the client connection.
   * @param request_str Full request string from the client.
   * @return 0 on success, -1 on error.
   */
  int handle_create(int sockfd, const std::string &request_str);

  /**
   * @brief Handles an FS_DELETE request from a client.
   * @param sockfd Socket file descriptor for the client connection.
   * @param request_str Full request string from the client.
   * @return 0 on success, -1 on error.
   */
  int handle_delete(int sockfd, const std::string &request_str);

  FileSystem my_filesystem;

  int server_fd;
  int port;
};
