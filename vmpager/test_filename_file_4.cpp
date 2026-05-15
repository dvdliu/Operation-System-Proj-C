#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    char* page0 = static_cast<char *>(vm_map(nullptr, 0));
    strcpy(page0, "data1.bin");

    char* filepage0 = static_cast<char *>(vm_map(page0, 0));
    strcpy(filepage0, "data2.bin"); // data2.bin lives in data1.bin
    
    /* Allocate swap-backed page from the arena */
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Evict the file page */
    strcpy(filename1, "skibidi1");
    strcpy(filename2, "skibidi2");
    strcpy(filename3, "skibidi3");
    strcpy(filename4, "skibidi4");

    printf("%s\n", filename1);
    printf("%s\n", filename2);
    printf("%s\n", filename3);
    printf("%s\n", filename4);

    printf("%s\n", filepage0);
    printf("%s\n", page0);
    strcpy(page0, filepage0);
    printf("%s\n", page0);
}