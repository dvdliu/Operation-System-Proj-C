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

  // TESTING BLANK USERNAME

  status = fs_create("", "/fortnite", 'd');
  assert(status);

  status = fs_writeblock("", "/fortnite", 0, writedata);
  assert(status);

  status = fs_readblock("", "/fortnite", 0, readdata);
  assert(status);

  status = fs_delete("", "/fortnite");
  assert(status);

  // TESTING WHITESPACE

  status = fs_create("us er1", "/fortnite", 'd');
  assert(status);

  status = fs_writeblock("us er1", "/fortnite", 0, writedata);
  assert(status);

  status = fs_readblock("us er1", "/fortnite", 0, readdata);
  assert(status);

  status = fs_delete("us er1", "/fortnite");
  assert(status);

  status = fs_create("us\ter1", "/fortnite", 'd');
  assert(status);

  status = fs_writeblock("us\ter1", "/fortnite", 0, writedata);
  assert(status);

  status = fs_readblock("us\ter1", "/fortnite", 0, readdata);
  assert(status);

  status = fs_delete("us\ter1", "/fortnite");
  assert(status);

  status = fs_create("user1", "/fortn ite", 'd');
  assert(status);

  status = fs_writeblock("user1", "/f ortnite", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "/fortni te", 0, readdata);
  assert(status);

  status = fs_delete("user1", "/for tnite");
  assert(status);

  // TESTING BLANK PATHNAME

  status = fs_create("user1", "", 'd');
  assert(status);

  status = fs_writeblock("user1", "", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "", 0, readdata);
  assert(status);

  status = fs_delete("user1", "");
  assert(status);

  // TESTING Null Bytes

  status = fs_create("use\0", "", 'd');
  assert(status);

  status = fs_writeblock("user1\0", "", 0, writedata);
  assert(status);

  status = fs_readblock("user1\0", "", 0, readdata);
  assert(status);

  status = fs_delete("user1\0", "");
  assert(status);

  // TESTING Pathname not starting with a /

  status = fs_create("user1", "dasda/das", 'd');
  assert(status);

  status = fs_writeblock("user1", "3asfrafa/sdfsdf", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "dfdsfsdf", 0, readdata);
  assert(status);

  status = fs_delete("user1", "aw4rwarwag");
  assert(status);

  // TESTING Pathname ending with a /

  status = fs_create("user1", "/root/", 'd');
  assert(status);

  status = fs_writeblock("user1", "/root/", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "/root/", 0, readdata);
  assert(status);

  status = fs_delete("user1", "/root/");
  assert(status);

  // TESTING Pathname whitespace

  status = fs_create("user1", "/r oot/", 'd');
  assert(status);

  status = fs_writeblock("user1", "/ro\tot/", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "/ro  ot/", 0, readdata);
  assert(status);

  status = fs_delete("user1", "\t/root/");
  assert(status);

  // TESTING Pathname double //

  status = fs_create("user1", "/root//hi", 'd');
  assert(status);

  status = fs_writeblock("user1", "/root//hi", 0, writedata);
  assert(status);

  status = fs_readblock("user1", "/root//hi", 0, readdata);
  assert(status);

  status = fs_delete("user1", "/root//hi");
  assert(status);

  // TESTING COMMAND SPECIFIC

  // type not f or d
  status = fs_create("user1", "/hi", 'a');
  assert(status);

  // type empty
  status = fs_create("user1", "/bye", '\0');
  assert(status);

  // super long?
  status = fs_create(
      "user1sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuse"
      "r1ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1s"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1ssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1sssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1ssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1sssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssuser1ssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssuser1sssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssuser1ssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssuser1sssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssuser1ssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssuser1sssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssuser1ssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssuser1sssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssuser1ssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssuser1sssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssuser1ssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssuser1sssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssuser1ssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssuser1sssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssuser1ssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssuser1sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssuser1ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "user1sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuse"
      "r1ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1s"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1ssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1sssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1ssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssuser1sssssssssssss"
      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      "sssssssssssssssssssssssssssssssssssssssssssssssssss",
      "/hi", 'd');
  assert(status);

  // testing write parsing
  // block leading 0
  status = fs_writeblock("user1", "/root", 00, writedata);
  assert(status);
  status = fs_writeblock("user1", "/root", 01, writedata);
  assert(status);

  // testing read parsing
  // block leading 0
  status = fs_readblock("user1", "/root", 00, readdata);
  assert(status);
  status = fs_readblock("user1", "/root", 01, readdata);
  assert(status);

  // test max_username over
  status = fs_create("abcdefghijk", "/fortnite", 'd');
  assert(status);

  status = fs_writeblock("abcdefghijk", "/fortnite", 0, writedata);
  assert(status);

  status = fs_readblock("abcdefghijk", "/fortnite", 0, readdata);
  assert(status);

  status = fs_delete("abcdefghijk", "/fortnite");
  assert(status);

  // test max_username under
  status = fs_create("abcdefghij", "/fortnite", 'f');
  assert(!status);

  // test max_filename over
  status = fs_create(
      "user1", "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefgh",
      'd');
  assert(status);

  status = fs_writeblock(
      "user1", "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefgh",
      0, writedata);
  assert(status);

  status = fs_readblock(
      "user1", "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefgh",
      0, readdata);
  assert(status);

  status = fs_delete(
      "user1", "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefgh");
  assert(status);

  // testing max_filename under
  status = fs_create(
      "user1", "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg",
      'd');
  assert(!status);

  // PREPARING TO test max_pathname over
  status = fs_create(
      "user1",
      "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/"
      "abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefghijklmnop",
      'd');
  assert(status);

  status =
      fs_create("user1",
                "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/"
                "abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg",
                'd');
  assert(!status);

  // testing maxpathanme over
  status = fs_create(
      "user1",
      "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/"
      "abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/hijklmno",
      'd');
  assert(status);

  // testing maxpathanme under
  status = fs_create(
      "user1",
      "/abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/"
      "abcdefghijklmnopqrstuvqxyzabcdefghijklmnopqrstuvqxyzabcdefg/hijklmn",
      'd');
  assert(!status);
}