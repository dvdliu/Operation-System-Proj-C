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

  status = fs_create("user1", "/dir", 'd');
  assert(!status);

  for(int i = 0; i < 10; i++) {
    std::string dir_name = "/dir/dir" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_create("user1", my_str, 'd');
    assert(!status);
  }

  for(int i = 0; i < 5; i++) {
    std::string dir_name = "/dir/dir1/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_create("user1", my_str, 'f');
    assert(!status);
  }

  for(int i = 0; i < 5; i++) {
    status = fs_writeblock("user1", "/dir/dir1/file1", i, writedata);
    assert(!status);
    status = fs_readblock("user1", "/dir/dir1/file1", i, readdata);
    assert(!status);
  }

  for(int i = 0; i < 5; i++) {
    std::string dir_name = "/dir/dir1/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_delete("user1", my_str);
    assert(!status);
  }

  status = fs_delete("user1", "/dir/dir1");
  assert(!status);

  for(int i = 0; i < 10; i++) {
    std::string dir_name = "/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_create("user2", my_str, 'f');
    assert(!status);
  }

  for(int i = 0; i < 10; i+=2) {
    std::string dir_name = "/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_delete("user2", my_str);
    assert(!status);
  }

  for(int i = 1; i < 10; i+=2) {
    std::string dir_name = "/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_writeblock("user2", my_str, 0, writedata);
    assert(!status);
  }

 
}