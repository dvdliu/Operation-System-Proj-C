#pragma once

class SocketRAII {
public:
  explicit SocketRAII(int sockfd);
  ~SocketRAII();

  // disables copy constructor / assignment operator
  // so we don't close same socket twice accidentally
  SocketRAII(const SocketRAII &) = delete;
  SocketRAII &operator=(const SocketRAII &) = delete;

private:
  int sockfd;
};