#include "socket_raii.h"
#include <cassert>
#include <unistd.h>

SocketRAII::SocketRAII(int socket_fd) : sockfd(socket_fd) {}

SocketRAII::~SocketRAII() {
  assert(sockfd != -1);
  close(sockfd);
}