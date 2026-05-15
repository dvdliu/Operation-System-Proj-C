#include "fs_client.h"
#include <cassert>
#include <string>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  char *server;
  int server_port;

  const char *writedata = "Skibidi fortnite hello skibidi gyatt gedagedigedagedayo";
  char readdata[FS_BLOCKSIZE];

  int status;

  if (argc != 3) {
    std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
    exit(1);
  }
  server = argv[1];
  server_port = atoi(argv[2]);

  fs_clientinit(server, server_port);

  std::string dir_name = "";
  for(int i = 0; i < 10; i++) {
    std::string file_name = dir_name + "/file";
    const char* my_file = file_name.c_str();
    dir_name += "/dir";
    const char* my_str = dir_name.c_str();
    status = fs_create("user1", my_str, 'd');
    assert(!status);
    status = fs_create("user1", my_file, 'f');
    assert(!status);
    status = fs_writeblock("user1", my_file, 0, writedata);
    assert(!status);
    status = fs_readblock("user1", my_file, 0, readdata);
    assert(!status);
  }

  status = fs_readblock("user1", "/dir/dir/dir/dir/file/dir/dir/dir/file", 0, readdata);
  assert(status);

  status = fs_readblock("use1", "/dir/dir/dir/dir/file/dir/dir/dir/file", 0, readdata);
  assert(status);
}