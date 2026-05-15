#include "vm_app.h"
#include <cstring>

// 2 pages of physical memory
// 4 swap blocks

int main() {
  // map two swap-backed pages
  auto *page0 = (char *)vm_map(nullptr, 0);
  auto *page1 = (char *)vm_map(nullptr, 0);
  auto *page2 = (char *)vm_map(nullptr, 0);
  auto *page3 = (char *)vm_map(nullptr, 0);

  std::strcpy(page0, "data1.bin");
  std::strcpy(page1, "data1.bin");
  std::strcpy(page2, "data1.bin");
  std::strcpy(page3, "data1.bin");

  // write the filename into virtual memory
  auto *filename = page0 + VM_PAGESIZE - 4;
  std::strcpy(filename, "data1.bin");

  // map a file-backed page
  vm_map(filename, 0);
}

// this test is testing when the filename is so long it flows out of physical
// memory