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

  status = fs_create("user1", "/dir", 'd');
  assert(!status);

  status = fs_create("user1", "/dir/file0", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file1", 'd');
  assert(!status);

  status = fs_create("user1", "/dir/file2", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file3", 'd');
  assert(!status);

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

  status = fs_create("user1", "/dir/file11", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file12", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file13", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file14", 'f');
  assert(!status);

  status = fs_delete("user1", "/dir/file2");
  assert(!status);

  status = fs_delete("user1", "/dir/file1");
  assert(!status);

  status = fs_delete("user1", "/dir/file9");
  assert(!status);

  status = fs_create("user1", "/dir/file2", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file1", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file9", 'f');
  assert(!status);

  status = fs_create("user1", "/dir/file17", 'f');
  assert(!status);

  status = fs_writeblock("user1", "/dir/file10", 0, writedata);
  assert(!status);

  status = fs_readblock("user1", "/dir/file10", 0, readdata);
  assert(!status);

  std::cout << readdata << std::endl;
}