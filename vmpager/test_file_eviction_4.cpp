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

    /* Map a page from the specified file */
    char* p = static_cast<char *>(vm_map (filename, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }
    cout << std::endl;

    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename2, "data2.bin");

    /* Map a page from the specified file */
    p = static_cast<char *>(vm_map (filename2, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }
    cout << std::endl;

    /* Allocate swap-backed page from the arena */
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename3, "data3.bin");

    /* Map a page from the specified file */
    p = static_cast<char *>(vm_map (filename3, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }
    cout << std::endl;

    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename4, "data4.bin");

    /* Map a page from the specified file */
    p = static_cast<char *>(vm_map (filename4, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	cout << p[i];
    }
    cout << std::endl;
}