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
    pages.resize(129);

    for(size_t i = 0; i < pages.size(); i++) {
        pages[i] = static_cast<char *>(vm_map(nullptr, 0));
    }

    fork();
}