// cl address_sanitizer_test.cpp /fsanitize=address /Zi /Od

#include <iostream>
#include <vector>

// cl address_sanitizer_test.cpp ippcore.lib ipps.lib /fsanitize=address /Zi /Od
#include "ipp.h"

int main()
{
    // // This shows up as stack-buffer-overflow
    // int y[10];
    // y[10] = 10; 

    // // This shows up as heap-buffer-overflow
    // int *x = (int*)malloc(sizeof(int) * 10);
    // x[10] = 10;

    // // This also shows up as heap-buffer-overflow
    // std::vector<int> v(10);
    // printf("v.capacity() = %zd\n", v.capacity());
    // v[10] = 10;

    // // IPP Mallocs do not show up as heap-buffer-overflow!
    // Ipp64s *z = ippsMalloc_64s_L(10);
    // printf("%p\n", z);
    // ippsZero_64s(z, 10);
    // z[10] = 10;
    // z[11] = 10;
    // z[15] = 10; // but if you go far enough out of bounds, then you'll get a heap-buffer-overflow?
    // ippsFree(z);

    Ipp64u *z = (Ipp64u*)ippsMalloc_64s(10);
    printf("%p\n", z);
    
    ippsZero_64s((Ipp64s*)z, 10);
    printf("%llX\n", *(z-1));
    printf("%llX\n", *(z+10));
    z[10] = 10;
    z[11] = 10;
    z[15] = 10; // but if you go far enough out of bounds, then you'll get a heap-buffer-overflow?
    ippsFree(z);

    /*
    The way IPP malloc works appears to be this:
    Allocate (requested bytes + 64 + 8) bytes.
    64 is for 64-byte alignment.
    8 bytes is clearly there to support the 'header' which points 
    to the OS-allocated memory block start,
    which is 8-bytes address for 64-bit computers.

    Then the function returns the earliest 64-byte aligned address in the block, and writes
    the start of the OS-block to that address-1.
    |...........|(addrstart, occupies 8 bytes)|     (user block, 64-byte aligned)    | ....... |
    ^
    this is addrstart
    */

    return 0;
}