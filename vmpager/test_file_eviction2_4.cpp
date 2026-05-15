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

    char* filepage1 = static_cast<char *>(vm_map(filepage0, 0));
    strcpy(filepage1, "data3.bin"); // data3.bin lives in data2.bin

    char* filepage2 = static_cast<char *>(vm_map(filepage1, 0));
    strcpy(filepage2, "data4.bin"); // data4.bin lives in data3.bin

    char* filepage3 = static_cast<char *>(vm_map(filepage2, 0));
    strcpy(filepage3, "I can't take it anymore"); 

    /* Allocate swap-backed page from the arena */
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Evict the file page */
    strcpy(filename1, "skibidi1");
    printf("%s\n", filepage0);
    strcpy(filename2, "skibidi2");
    printf("%s\n", page0);

    strcpy(filepage0, "skibidi1");
    strcpy(filepage1, "data4.bin");
    strcpy(filepage2, "data3.bin");
    printf("%s\n", filepage0);
    printf("%s\n", page0);

    strcpy(filename3, "skibidi3");
    printf("%s\n", filepage1);
    printf("%s\n", filepage3);
    strcpy(filename4, "skibidi4");
    printf("%s\n", filepage2);
    strcpy(filepage2, "What the sigma");

    printf("%s\n", filepage0);
    printf("%s\n", page0);
    strcpy(page0, filepage0);
    printf("%s\n", filepage1);
    printf("%s\n", filepage2);
    printf("%s\n", page0);
    strcpy(filepage3, "I want to die haha");
    printf("%s\n", filepage3);
}