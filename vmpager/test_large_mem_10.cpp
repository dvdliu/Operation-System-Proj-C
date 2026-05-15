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
    size_t num_pages = 128;
    pages.resize(num_pages);

    vector<char *> file_pages;
    file_pages.resize(128);

    for(size_t i = 0; i < num_pages; i++) {
        pages[i] = static_cast<char *>(vm_map(nullptr, 0));
    }

    for(size_t i = 0; i < num_pages; i++){
        strcpy(pages[i], "data1.bin");
    }

    file_pages[0] = static_cast<char *>(vm_map(pages[0], 0));
    file_pages[1] = static_cast<char *>(vm_map(pages[100], 1));

    strcpy(file_pages[0], "SKIBIDI SKIBID     SKIBID");
    printf("%s\n", file_pages[0]);

    strcpy(file_pages[1], "I hate my life");
    printf("%s\n", file_pages[1]);

    printf("Done writing to filepages\n");

    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename2 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename3 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename4 = static_cast<char *>(vm_map(nullptr, 0));

    /* Write the name of the file that will be mapped */
    strcpy(filename1, "skibidi1");
    strcpy(filename2, "skibidi2");
    strcpy(filename3, "skibidi3");
    strcpy(filename4, "skibidi4");

    printf("%s\n", filename1);
    printf("%s\n", filename2);
    printf("%s\n", filename3);
    printf("%s\n", filename4);

    strcpy(filename2, "skibidi5");
    strcpy(filename3, "skibidi6");
    strcpy(filename1, "skibidi7");
    strcpy(filename4, "skibidi8");

    printf("%s\n", filename2);
    printf("%s\n", filename3);
    printf("%s\n", filename1);
    printf("%s\n", filename4);
}