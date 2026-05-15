#include "fs_client.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

void read(int range) {
  int status;
  char readdata[FS_BLOCKSIZE];

  std::cout << "\nREADING FROM BLOCK 0 TO BLOCK " << range << ":\n[";

  for (int i = 0; i <= range; ++i) {
    status = fs_readblock("user1", "/dir/file", i, readdata);
    assert(!status);

    std::cout << readdata << ", ";
  }

  std::cout << "]\n\n\n";
}

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

  const char *mywritedata =
      "I DONT WANT A LOT FOR CRHIMAS THERE IS JUST ONE THING I NEED AND I DONT "
      "CARE ABOUT THE PRESENTS UNDERNEATH THE CHRIMAS TREE I JUST WANT YOU FOR "
      "MY OWN MORE THAN YOU WILL EVER KNOW, MAKE MY WISH COME TRUEEEEE CUZ "
      "BABY ALL I WANT FOR CHRIMAS IS YOUUU";

  const char *onebytedata = "";

  const char *twobytedata = "a";

  // 512 bytes
  const char *oneblockdata =
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE FORTNITE "
      "FORTNITE FORTNIT";

  // 513 bytes
  const char *oneblockplusonedata =
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI SKIBIDI "
      "SKIBIDIS";

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

  // []

  // tests extending writeblock
  status = fs_writeblock("user1", "/dir/file", 0, writedata);
  assert(!status);

  // ["specstuff"]
  read(0);

  status = fs_writeblock("user1", "/dir/file", 1, writedata);
  assert(!status);

  // ["specstuff", "specstuff"]
  read(1);

  // testing overwriting a block
  status = fs_writeblock("user1", "/dir/file", 0, mywritedata);
  assert(!status);

  // ["chrimas", "specstuff"]
  read(1);

  // writing exactly 512 bytes (one block)
  status = fs_writeblock("user1", "/dir/file", 0, oneblockdata);
  assert(!status);
  status = fs_writeblock("user1", "/dir/file", 2, oneblockdata);
  assert(!status);

  // ["fortnite", "specstuff", "fortnite"]
  read(2);

  status = fs_writeblock("user1", "/dir/file", 0, onebytedata);
  assert(!status);

  // ["", "specstuff", "fortnite"]
  read(2);

  status = fs_writeblock("user1", "/dir/file", 0, twobytedata);
  assert(!status);

  // ["a", "specstuff", "fortnite"]
  read(2);

  // writing exactly 513 bytes (one block + 1)
  status = fs_writeblock("user1", "/dir/file", 0, oneblockplusonedata);
  assert(!status);
  status = fs_writeblock("user1", "/dir/file", 1, oneblockplusonedata);
  assert(!status);
  status = fs_writeblock("user1", "/dir/file", 2, oneblockplusonedata);
  assert(!status);
  status = fs_writeblock("user1", "/dir/file", 3, oneblockplusonedata);
  assert(!status);

  read(3);
}