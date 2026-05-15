#include "vm_app.h"
#include <cstring>

// 2 pages of physical memory
// 4 swap blocks

int main() {
  // map one swap-backed pages
  auto *page0 = (char *)vm_map(nullptr, 0);

  // write the filename into virtual memory
  auto *filename = page0 + VM_PAGESIZE - 1;
  std::strcpy(filename, "data1.bin");
  // map a file-backed page
  vm_map(filename, 0);
}

// this test case should segfault as it is trying to write to uninitalized
// memory (our pager should not segfault though)
// our page should properly handle this by returning -1 for vmfault (i think???)