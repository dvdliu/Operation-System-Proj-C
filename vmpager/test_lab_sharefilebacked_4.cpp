#include "vm_app.h"
#include <cassert>
#include <cstring>
#include <unistd.h>

int main() { /* 4 pages of physical memory in the system */
  auto *filename = static_cast<char *>(vm_map(nullptr, 0));
  std::strcpy(filename, "data1.bin");

  if (fork()) { // parent
    // setting up the mapping did not require us to read the block in
    auto *fb_page = static_cast<char *>(vm_map(filename, 0));

    // Should this write fault? -> no because we just mapped it and didnt read
    // it into physmem
    fb_page[0] = 'B';
    vm_yield();
  } else { // child
    auto *fb_page = static_cast<char *>(vm_map(filename, 0));

    // next two should NOT fault since it is in physmem from parent
    assert(fb_page[0] == 'B');
    fb_page[0] = 'H';
  }
}
