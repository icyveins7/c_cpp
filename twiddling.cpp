// compile with -march=native to enable the intrinsics

#include <iostream>
#include <immintrin.h>
#include <stdint.h>
#include <vector>
#include <bitset>
#include "ipp.h"
#include "ipp_ext.h"

#include "timer.h"

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
void gray_qpsk_rotate(uint8_t *bits, uint8_t *out, int length)
{
    uint8_t mask;
    for (int i = 0; i < length; i++)
    {
        mask = (bits[i] ^ (bits[i] >> 1)) & 0x55;
        mask |= (~(mask << 1) & 0xaa);
        
        out[i] = bits[i] ^ mask;
    }
}

void gray_qpsk_rotate_Ipp8u(Ipp8u *bits, Ipp8u *out, int length)
{
    ippe::vector<Ipp8u> mask;
    ippe::vector<Ipp8u> nmask;
    mask.resize(length);
    nmask.resize(length);

    ippsRShiftC_8u(bits, 1, mask.data(), length); // bits >> 1
    ippsXor_8u_I(bits, mask.data(), length); // bits ^ (bits >> 1)
    ippsAndC_8u_I(0x55, mask.data(), length); // (bits ^ (bits >> 1)) & 0x55
    ippsLShiftC_8u(mask.data(), 1, nmask.data(), length); // mask << 1
    ippsNot_8u_I(nmask.data(), length); // ~(mask << 1)
    ippsAndC_8u_I(0xaa, nmask.data(), length); // (~(mask << 1) & 0xaa)
    ippsOr_8u_I(nmask.data(), mask.data(), length); // mask |= (~(mask << 1) & 0xaa)

    ippsXor_8u(bits, mask.data(), out, length);
}

void gray_qpsk_rotate32(uint32_t *bits, uint32_t *out, int length)
{
    uint32_t mask, tmp;
    for (int i = 0; i < length; i++)
    {
        mask = (bits[i] ^ (bits[i] >> 1)) & 0x55555555;
        mask |= (~(mask << 1) & 0xaaaaaaaa);
        
        // DEPRECATED. DO NOT USE THIS. ORDERS OF MAGNITUDE SLOWER.
        // tmp = _pext_u32(bits[i], 0x55555555) ^ _pext_u32(bits[i], 0xaaaaaaaa); // deposits to lower/rightmost 16 bits
        // mask = 0;
        // mask |= _pdep_u32(tmp, 0x55555555); // deposit the first bits
        // mask |= _pdep_u32(~tmp, 0xaaaaaaaa); // deposit the second bits, which are the NOT of the first

        out[i] = bits[i] ^ mask;
    }
}

void gray_qpsk_rotate64(uint64_t *bits, uint64_t *out, int length)
{
    uint64_t mask, tmp;
    for (int i = 0; i < length; i++)
    {
        mask = (bits[i] ^ (bits[i] >> 1)) & 0x5555555555555555;
        mask |= (~(mask << 1) & 0xaaaaaaaaaaaaaaaa);

        // DEPRECATED. DO NOT USE THIS. ORDERS OF MAGNITUDE SLOWER.
        // tmp = _pext_u64(bits[i], 0x5555555555555555) ^ _pext_u64(bits[i], 0xaaaaaaaaaaaaaaaa); // deposits to lower/rightmost 16 bits
        // mask = 0;
        // mask |= _pdep_u64(tmp, 0x5555555555555555); // deposit the first bits
        // mask |= _pdep_u64(~tmp, 0xaaaaaaaaaaaaaaaa); // deposit the second bits, which are the NOT of the first

        out[i] = bits[i] ^ mask;
    }
}

/////////////////////////////////////////
int main(int argc, char *argv[])
{
    {
        // Test scenario
        uint32_t bits = 0b11010010110100101101001011010010;
        uint32_t out;
        gray_qpsk_rotate32(&bits, &out, 1);
        
        uint8_t* bits8 = (uint8_t*)&bits;
        uint8_t* out8 = (uint8_t*)&out;

        std::cout << std::bitset<8>(bits8[0]) << std::endl;
        std::cout << std::bitset<8>(out8[0]) << std::endl;

        const uint32_t expected = 0b01001011010010110100101101001011;
        if (out == expected){printf("Verified.\n");}
        else{printf("Error.\n");}
    }

    {
        // IPP version correctness
        Ipp8u bits = 0b11010010;
        Ipp8u out1, out2;
        gray_qpsk_rotate(&bits, &out1, 1);
        gray_qpsk_rotate_Ipp8u(&bits, &out2, 1);

        std::cout << std::bitset<8>(bits) << std::endl;
        std::cout << std::bitset<8>(out1) << std::endl;
        std::cout << std::bitset<8>(out2) << std::endl;

        const Ipp8u expected = 0b01001011;
        if (out1 == expected && out2 == expected){printf("IPP Verified.\n");}
        else{printf("IPP Error.\n");}
    }

    {
        // 8-bit speed is slower than the 32-bit and 64-bit versions, even for total number of bits being equal
        const size_t length = 40000000;
        printf("Length %zd 8-bit array.\n", length);
        std::vector<uint8_t> bits(length); 
        std::vector<uint8_t> out(length);
        HighResolutionTimer timer;
        gray_qpsk_rotate(bits.data(), out.data(), length);

    }

    
    {
        // IPP 8u Speed (this is slower than default function! probably due to extra allocations and bad memory caching)
        const size_t length = 40000000;
        printf("Length %zd 8-bit Ipp array.\n", length);
        ippe::vector<Ipp8u> bits(length); 
        ippe::vector<Ipp8u> out(length);
        HighResolutionTimer timer;
        gray_qpsk_rotate_Ipp8u(bits.data(), out.data(), length);
    }

    {
        const size_t length = 10000000;
        printf("Length %zd 32-bit array.\n", length);
        std::vector<uint32_t> bits(length); 
        std::vector<uint32_t> out(length);
        HighResolutionTimer timer;
        gray_qpsk_rotate32(bits.data(), out.data(), length);

    }

    

    {
        // 64-bit speed is roughly the same as 32-bit speed
        const size_t length = 5000000;

        printf("Length %zd 64-bit array.\n", length);
        std::vector<uint64_t> bits(length); 
        std::vector<uint64_t> out(length);
        HighResolutionTimer timer;
        gray_qpsk_rotate64(bits.data(), out.data(), length);
    }
    

    return 0;
}