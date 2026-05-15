#include "fs_client.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  char *server;
  int server_port;

  char readdata[FS_BLOCKSIZE];
  int status;

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

  if (argc != 3) {
    std::cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
    exit(1);
  }
  server = argv[1];
  server_port = atoi(argv[2]);

  fs_clientinit(server, server_port);

  status = fs_delete("user2", "");
  assert(status);

  status = fs_create("user1", "/dir", 'd');
  assert(!status);

  status = fs_create("user1", "/file1", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file1", 'f');
  assert(!status);

  status = fs_delete("user1", "/dir");
  assert(status);

  status = fs_create("user2", "/dir/file1", 'f');
  assert(status);

  status = fs_delete("user2", "/dir/file1");
  assert(status);

  status = fs_delete("user2", "");
  assert(status);

  status = fs_delete("user2", "/");
  assert(status);

  status = fs_create("user2", "", 'd');
  assert(status);

  status = fs_create("user2", "/", 'd');
  assert(status);

  status = fs_readblock("user2", "/file1", 0, readdata);
  assert(status);

  status = fs_writeblock("user2", "/file1", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "/dir", 0, readdata);
  assert(status);

  status = fs_create("user1", "/dir/file0", 'f');
  assert(!status);

  status = fs_create("user1", "/dir", 'f');
  assert(status); // path already exists

  status = fs_create("user1", "/dir/file17", 'd');
  assert(!status);

  status = fs_create("user1", "/dir/file1", 'f');
  assert(status); // error, duplicate create

  status = fs_create("user1", "/dir/file2", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file3", 'd');
  assert(!status);

// read and write to folder
  status = fs_readblock("user1", "/dir/file3", 0, readdata);
  assert(status);

  status = fs_writeblock("user1", "/dir/file3", 0, writedata);
  assert(status);
// errors ^

  status = fs_create("user1", "/dir/file4", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file5", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file6", 'd');
  assert(!status);

  status = fs_create("user1", "/dir/file7", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file8", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file9", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file10", 'f');
  assert(!status);

// Does not exist errors
  status = fs_delete("user1", "/dir/file3/f");
  assert(status);

  status = fs_readblock("user1", "/dir/file3/f", 0, readdata);
  assert(status);

  status = fs_writeblock("user1", "/dir/file9", 1, writedata);
  assert(status);

  status = fs_create("user1", "/dir/file1/hello/wrong", 'f');
  assert(status);

// out of bounds

  status = fs_writeblock("user1", "/dir/file0", 0, writedata);
  assert(!status);

  status = fs_writeblock("user1", "/dir/file0", 2, writedata);
  assert(status); // out of bounds error

// no permissions

  status = fs_readblock("user2", "/dir/file0", 0, readdata);
  assert(status);

  status = fs_writeblock("user2", "/dir/file0", 1, writedata);
  assert(status);

  status = fs_delete("user2", "/dir/file0");
  assert(status);

  status = fs_create("user1", "/dir/file0/file", 'f');
  assert(status);


  std::cout << readdata << std::endl;
}