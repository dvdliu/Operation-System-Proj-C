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

  status = fs_create("user1", "/dir", 'd');
  assert(!status);

  for(int i = 0; i < 16; i++) {
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
    std::string dir_name = "/dir/dir1/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_delete("user1", my_str);
    assert(!status);
  }

  status = fs_delete("user1", "/dir/dir1");
  assert(!status);

  for(int i = 0; i < 16; i++) {
    std::string dir_name = "/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_create("user2", my_str, 'f');
    assert(!status);
  }

  for(int i = 0; i < 16; i++) {
    std::string dir_name = "/file" + std::to_string(i);
    const char* my_str = dir_name.c_str();
    status = fs_delete("user2", my_str);
    assert(!status);
  }

 
}