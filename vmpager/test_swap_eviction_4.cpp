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

    /* Correct output:
    After line 13, 4 virtual pages with read access to ppage 0
    Lines 16, 17, and 18 should all trigger write-faults and create copies with ppages 1, 2, and 3
    Line 19 causes an eviction, which should evict virtual page 0 and the clock queue should have pages 1, 2, 3

    At this point, only virtual page 3 should have rw bits, as the others need to be marked resident
    Lines 21 and 22 should trigger a vm fault, where we mark virtual page 1 and 2 as read/write enabled
    Line 23 should trigger a fault and cause an eviction, where virtual page 1 should be evicted and virtual page 0 to come in with ppage 2
    clock queue = 2 (not referenced), 3 (not referenced), 0 (referenced)

    Line 24 should mark virtual page 3 as referenced and it should have ppage 1
    */
}