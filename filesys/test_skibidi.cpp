#include "fs_client.h"
#include <cassert>
#include <string>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  char *server;
  int server_port;

  int status;

  if (argc != 3) {
    std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
    exit(1);
  }
  server = argv[1];
  server_port = atoi(argv[2]);

  fs_clientinit(server, server_port);

  status = fs_create("u", "/w", 'f');
  assert(status);

  status = fs_create("user1", "/dir/hello", 'f');
  assert(!status);
}