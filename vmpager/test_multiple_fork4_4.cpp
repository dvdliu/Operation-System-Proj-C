#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;
using std::endl;

int main() {
    /* Allocate swap-backed page from the arena */
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename1, "data4.bin");
    strcpy(filename2, "data2.bin");
    strcpy(filename3, "data3.bin");
    strcpy(filename4, "data1.bin");

    char* filepage1 = static_cast<char *>(vm_map(filename1, 0));
    char* filepage2 = static_cast<char *>(vm_map(filename2, 0));
    char* filepage3 = static_cast<char *>(vm_map(filename3, 0));
    char* filepage4 = static_cast<char *>(vm_map(filename4, 0));

    for (unsigned int i=0; i<10; i++) {
	cout << filepage1[i];
    }
    cout << endl;

    for (unsigned int i=0; i<10; i++) {
	cout << filepage2[i];
    }
    cout << endl;

    for (unsigned int i=0; i<10; i++) {
	cout << filepage3[i];
    }
    cout << endl;

    for (unsigned int i=0; i<10; i++) {
	cout << filepage4[i];
    }
    cout << endl;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        cout << "Child process:\n";
    } else {
        // Parent process
        cout << "Parent process:\n";
    }

    strcpy(filename1, "data4.bin");
    strcpy(filename2, "data2.bin");

    for (unsigned int i=0; i<10; i++) {
	cout << filepage4[i];
    }
    cout << endl;
    printf("%s\n", filename3);
    printf("%s\n", filename4);
    for (unsigned int i=0; i<10; i++) {
	cout << filepage3[i];
    }
    cout << endl;
    printf("%s\n", filename1);
    printf("%s\n", filename2);

    // This should copy on write for the parent process
    strcpy(filename2, "skibidi5");
    strcpy(filename3, "skibidi6");
    strcpy(filename1, "skibidi7");
    pid = fork();
    if (pid == 0) {
        // Child process
        cout << "Child process:\n";
    } else {
        // Parent process
        cout << "Parent process:\n";
    }
    strcpy(filename4, "skibidi8");

    printf("%s\n", filename2);
    printf("%s\n", filename3);
    printf("%s\n", filename1);
    printf("%s\n", filename4);
}