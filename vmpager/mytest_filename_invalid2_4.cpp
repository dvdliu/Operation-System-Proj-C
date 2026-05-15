#include "vm_app.h"
#include <cstring>

// 2 pages of physical memory
// 4 swap blocks

int main() {
  // map two swap-backed pages
  auto *page0 = (char *)vm_map(nullptr, 0);

  // write the filename into virtual memory
  auto *filename = page0 + VM_PAGESIZE - 4;
  std::strcpy(filename, "lampson83.txt");

  // map a file-backed page
  vm_map(filename, 0);
}

// this test case should segfault as it is trying to write to uninitalized
// memory (our pager should not segfault though)
// our page should properly handle this by returning -1 for vmfault (i think???)