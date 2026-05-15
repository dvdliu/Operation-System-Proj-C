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

    char* p = static_cast<char *>(vm_map (filename, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<100; i++) {
	cout << p[i];
    }
    cout << std::endl;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        cout << "Child process:\n";
    } else {
        // Parent process
        cout << "Parent process:\n";
    }
}