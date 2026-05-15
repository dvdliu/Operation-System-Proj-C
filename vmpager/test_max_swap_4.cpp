#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <string>
#include "vm_app.h"

using std::vector;
using std::cout;
using std::string;

int main() {

    vector<char *> pages;
    pages.resize(100);

    for(size_t i = 0; i < pages.size(); i++) {
        pages[i] = static_cast<char *>(vm_map(nullptr, 0));
    }

    for(size_t i = 0; i < pages.size(); i++) {
        string skib = "skibidi" + std::to_string(99 - i);
        strcpy(pages[99 - i], skib.c_str());
    }

    fork(); // can comment this out for 4-credit

    vm_map(nullptr, 0);

    char* filename = pages[0];

    // /* Write the name of the file that will be mapped */
    strcpy(filename, "data1.bin");

    printf("%s\n", filename);

    // /* Map a page from the specified file */
    vm_map (filename, 0); // vm_map should return nullptr bc arena is full
}