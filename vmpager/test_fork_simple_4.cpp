#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    /* Allocate swap-backed page from the arena */
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename5 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    vm_map(filename1, 0);
    strcpy(filename1, "data2.bin");
    strcpy(filename2, "skibidi2");
    char* p = static_cast<char *>(vm_map(filename1, 1));
    strcpy(filename3, "skibidi3");
    strcpy(filename4, "skibidi4");
    strcpy(filename5, "skibidi5");

    vm_map(filename1, 0);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        cout << "Child process:\n";
    } else {
        // Parent process
        cout << "Parent process:\n";
    }


    printf("%s\n", filename1);
    printf("%s\n", filename2);
    printf("%s\n", filename3);
    strcpy(filename1, "skibidi1");
    strcpy(filename2, "skibidi2");
    strcpy(filename3, "skibidi3");
    strcpy(p, "Burger king whopper");
    strcpy(filename4, "skibidi4");
    strcpy(filename5, "skibidi5");
    printf("%s\n", filename4);
    printf("%s\n", filename5);

    fork();
    vm_map(nullptr, 0);
}