#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    /* Allocate swap-backed page from the arena */
    char* filename = static_cast<char *>(vm_map(nullptr, 0));
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename, "data1.bin");
    char* p = static_cast<char *>(vm_map (filename, 0));

    strcpy(p, "data1.bin");
    for (unsigned int i=0; i<10; i++) {
    cout << p[i];
    }
    strcpy(filename1, "data1.bin");
    strcpy(filename2, "data1.bin");
    strcpy(filename3, "data1.bin");

    p = static_cast<char *>(vm_map (filename, 0));
    strcpy(p, "I can't take it anymore");

    if (fork()) {
        /* Map a page from the specified file */

        cout << "Parent\n";
        /* Print the first part of the paper */
        for (unsigned int i=0; i<10; i++) {
        cout << p[i];
        }
        cout << std::endl;
        strcpy(p, "PLEASE PLEASE PLEASE CATCH A BUG");
    } else {

        /* Map a page from the specified file */

        cout << "Child\n";
        /* Print the first part of the paper */
        for (unsigned int i=0; i<10; i++) {
        cout << p[i];
        }
        cout << std::endl;
        strcpy(p, "PLEASE PLEASE PLEASE CATCH A BUG");
    }
}