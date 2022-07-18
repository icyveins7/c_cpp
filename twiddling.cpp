// compile with -march=native to enable the intrinsics

#include <iostream>
#include <immintrin.h>
#include <stdint.h>
#include <array>
#include <bitset>

/*
This function rotates the QPSK gray-coded bits in a counter-clockwise fashion.
Notably, the gray-coded QPSK symbols correspond as
exp(i pi/4) = 11
exp(i 3pi/4) = 01
exp(i 5pi/4) = 00
exp(i 7pi/4) = 10

The rotation in question is meant to map the bit sequences to the next one in the list i.e.
11 -> 01
01 -> 00
00 -> 10
10 -> 11

Since the gray-coding induces only 1 bit flip each time, what remains is to find out which bit to flip.
It turns out this is given by the XOR of both bits
XOR == 0 -> bit 0 flips == XOR with 10 
XOR == 1 -> bit 1 flips == XOR with 01
*/
template<std::size_t SIZE>
void gray_qpsk_rotate(std::array<uint32_t, SIZE> &bits, std::array<uint32_t, SIZE> &out)
{
    uint32_t mask, tmp;
    for (int i = 0; i < SIZE; i++)
    {
        tmp = _pext_u32(bits[i], 0x55555555) ^ _pext_u32(bits[i], 0xaaaaaaaa); // deposits to lower/rightmost 16 bits
        mask = 0;
        mask = mask | _pdep_u32(tmp, 0x55555555); // deposit the first bits
        mask = mask | _pdep_u32(~tmp, 0xaaaaaaaa); // deposit the second bits, which are the NOT of the first

        out[i] = bits[i] ^ mask;
    }
}

int main(int argc, char *argv[])
{
    std::array<uint32_t, 1> bits = {0b11010010110100101101001011010010};
    std::array<uint32_t, 1> out;

    gray_qpsk_rotate(bits, out);

    std::cout << std::bitset<32>(bits[0]) << std::endl;
    std::cout << std::bitset<32>(out[0]) << std::endl;


    return 0;
}