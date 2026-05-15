#include "fs_client.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  char *server;
  int server_port;

  const char *writedata =
      "We hold these truths to be self-evident, that all men are created "
      "equal, that they are endowed by their Creator with certain unalienable "
      "Rights, that among these are Life, Liberty and the pursuit of "
      "Happiness. -- That to secure these rights, Governments are instituted "
      "among Men, deriving their just powers from the consent of the governed, "
      "-- That whenever any Form of Government becomes destructive of these "
      "ends, it is the Right of the People to alter or to abolish it, and to "
      "institute new Government, laying its foundation on such principles and "
      "organizing its powers in such form, as to them shall seem most likely "
      "to effect their Safety and Happiness.";

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

  status = fs_create("user1", "/dir/file", 'f');
  assert(!status);

  status = fs_create("user1", "/file", 'f');
  assert(!status);

  status = fs_create("user2", "/dir/skibidi", 'f');
  assert(status);

  status = fs_create("user2", "/dir/fortnite", 'd');
  assert(status);

  // user 2 trying to write to user 1s file
  status = fs_writeblock("user2", "/file", 0, writedata);
  assert(status);

  status = fs_writeblock("user1", "/file", 0, writedata);
  assert(!status);

  // user 2 trying to write to user 1s dir
  status = fs_writeblock("user2", "/dir/file", 0, writedata);
  assert(status);

  status = fs_writeblock("user1", "/dir/file", 0, writedata);
  assert(!status);

  // user 2 trying to read from user 1s file
  status = fs_readblock("user2", "/file", 0, readdata);
  assert(status);

  // user 2 trying to read user 1's dir
  status = fs_readblock("user2", "/dir/file", 0, readdata);
  assert(status);

  // user 2 trying to delete user 1's file
  status = fs_delete("user2", "/dir/file");
  assert(status);

  status = fs_delete("user2", "/file");
  assert(status);

  status = fs_delete("user1", "/dir/file");
  assert(!status);

  // user 2 trying to delete user 1s dir
  status = fs_delete("user2", "/dir");
  assert(status);

  status = fs_delete("user1", "/dir");
  assert(!status);
}