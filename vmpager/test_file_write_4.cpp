#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    /* Allocate swap-backed page from the arena */
    char* filename = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename, "data1.bin");

    printf("%s\n", filename);

    /* Map a page from the specified file */
    char* p = static_cast<char *>(vm_map (filename, 0));

    strcpy(p, "i love this class haha this project is really fun");

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }

    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename1, "skibidi1");
    strcpy(filename2, "skibidi2");
    strcpy(filename3, "skibidi3");
    strcpy(filename4, "skibidi4");
}