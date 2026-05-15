#include "fs_client.h"
#include <cassert>
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

  // creating the basic layout
  status = fs_create("user1", "/dir", 'd');
  assert(!status);

  status = fs_create("user1", "/file", 'f');
  assert(!status);

  // testing for creating duplicate over a directory
  status = fs_create("user1", "/dir", 'd');
  assert(status);

  status = fs_create("user1", "/dir", 'f');
  assert(status);

  status = fs_create("user2", "/dir", 'd');
  assert(status);

  status = fs_create("user2", "/dir", 'f');
  assert(status);

  // testing for creating duplicate over file
  status = fs_create("user1", "/file", 'd');
  assert(status);

  status = fs_create("user1", "/file", 'f');
  assert(status);

  status = fs_create("user2", "/file", 'd');
  assert(status);

  status = fs_create("user2", "/file", 'f');
  assert(status);
}