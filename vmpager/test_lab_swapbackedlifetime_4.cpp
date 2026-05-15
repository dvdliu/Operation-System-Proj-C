#include "vm_app.h"
#include <cstring>
#include <unistd.h>

int main() {
  /* 4 pages of physical memory in the system */
  if (fork() != 0) { // parent
    auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
    auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
    auto *page2 = static_cast<char *>(vm_map(nullptr, 0));
    page0[0] = page1[0] = page2[0] = 'a';
    // How many physical pages are free just before the parent exits? -> none!
    // Just after? -> 3 pages free!
  } else { // child
    auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
    std::strcpy(page0, "Hello, world!"); // does not need any evictions
  }
}
