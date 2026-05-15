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
    size_t num_pages = 127;
    pages.resize(num_pages);

    for(size_t i = 0; i < num_pages; i++) {
        pages[i] = static_cast<char *>(vm_map(nullptr, 0));
    }

    fork();

    vector<char *> pages1;
    pages1.resize(5);
    for(size_t i = 0; i < 5; i++) {
        pages1[i] = static_cast<char *>(vm_map(nullptr, 0));
    }
}