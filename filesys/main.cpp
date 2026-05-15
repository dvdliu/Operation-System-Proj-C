#include "network_server.h"

int main(int argc, char *argv[]) {
  int port = 0;

  if (argc == 2) {
    port = atoi(argv[1]);
  }

  try {
    NetworkServer my_server(port);

    my_server.run();
  } catch (...) {
    return -1;
  }
}