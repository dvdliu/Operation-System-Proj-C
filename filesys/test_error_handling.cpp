#include "fs_client.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  char *server;
  int server_port;

  char readdata[FS_BLOCKSIZE] {};
  int status;

  if (argc != 3) {
    std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
    exit(1);
  }
  server = argv[1];
  server_port = atoi(argv[2]);

  fs_clientinit(server, server_port);

  status = fs_readblock("invalid", "/file", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/nooooooooooooooooooooooooooooooooooooooooooooooothing", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "nothing/nothing", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/nothing", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/dir", 0, readdata);
  assert(status);

  status = fs_readblock("skibidi_ohio_gyatt_69", "/file", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/dir", 0, readdata);
  assert(status);

  status = fs_readblock("user1", "/file", 5, readdata);
  assert(status);

  status = fs_readblock("user1", "/file", FS_MAXFILEBLOCKS + 1, readdata);
  assert(status);
}