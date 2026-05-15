#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    /* Allocate swap-backed page from the arena */
    char* filename = static_cast<char *>(vm_map(nullptr, 0));

    char* invalid = filename + 0x0999;

    /* Write the name of the file that will be mapped */
    strcpy(invalid, "data1.bin");

    /* Map a page from the specified file */
    char* p = static_cast<char *>(vm_map (filename, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }
}