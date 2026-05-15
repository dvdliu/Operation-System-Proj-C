#include "vm_app.h"
#include <cstring>

// 2 pages of physical memory
// 4 swap blocks

int main() {
  // map two swap-backed pages
  auto *page0 = (char *)vm_map(nullptr, 0);

  // write the filename into virtual memory without the null-terminating byte
  auto *filename = page0 + VM_PAGESIZE - 1;
  const char *name = "a";
  // doesnt copy null term char!
  std::memcpy(filename, name, std::strlen(name));

  // map a file-backed page
  vm_map(filename, 0);
}

// this test should have vm_map return nullptr because it is reading out of
// bounds of mapped virtual pages