// g++ twiddling.cpp -lippcore -lipps -o twiddling -march=native
// cl twiddling.cpp ippcore.lib ipps.lib /EHsc /O2 /arch:AVX2

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

        out[i] = bits[i] ^ mask;
    }
}

inline uint8_t extractBitSequence(uint8_t *src, int len, int bitIdx)
{
    if (bitIdx > len*8){throw -1;}
    else{
        int sb = bitIdx / 8;
        int sm = bitIdx % 8;
        if (sb == len - 1){ return src[sb] << sm; }
        else{ return (src[sb] << sm) | (src[sb+1] >> (8-sm)); }
    }
}

std::vector<uint32_t> match_amble(uint8_t *src, int srclen, uint8_t *amble, int amblelen, uint8_t *amblemask)
{
    printf("Amblelen/masklen (in bytes) = %d\n", amblelen);

    int amblebitlen = 0;
    int iters = amblelen / 4;
    int rem = amblelen % 4;
    uint32_t *ambleptr;
    for (int i = 0; i < iters; i++){
        ambleptr = (uint32_t*)&amblemask[i*4];
        amblebitlen += _mm_popcnt_u32(*ambleptr);
    }
    if (rem > 0)
    {
        uint32_t rembits = 0;
        for (int i = 0; i < rem; i++)
        {
            rembits |= (amblemask[iters*4+i] << (3-i)*8);
        }
        amblebitlen += _mm_popcnt_u32(rembits);
    }
    printf("Amblelen/masklen (in bits) = %d\n", amblebitlen);

    // Check how many we need to iterate
    int numSearch = srclen*8 - amblebitlen + 1;
    printf("Source is %d bits, search length is %d \n", srclen*8, numSearch);
    int sb, sm;
    uint8_t srcByte;
    uint8_t err;
    std::vector<uint32_t> errs(numSearch);
    uint32_t xorout;

    // Loop over the starting source bit
    for (int s = 0; s < numSearch; s++)
    {
        sb = s / 8; // This is the starting byte
        sm = s % 8; // This is the starting bit
        err = 0;

        // Internal loop over the the amble
        for (int a = 0; a < amblelen; a++)
        {
            srcByte = src[sb]; // By default its just this
            if (sm != 0) // If the bit shift is non-zero we must take part of the next byte
            {
                srcByte = (srcByte << sm) | (src[sb+1] >> (8-sm));
            }

            // Now compare this to the amble value
            xorout = (srcByte ^ amble[a]) & amblemask[a]; // This leaves 1-bits in spots that don't match
            err += _mm_popcnt_u32(xorout); // TODO: highly inefficient as we only occupy 8 out of 32 bits
        }
        errs.at(s) = err;

    }

    return errs;

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

        // Test amble matching
        uint8_t amblemask[1] = {0b11111100};

        std::vector<uint32_t> amble_errs = match_amble(out8, 4, out8, 1, amblemask);

        for (int i = 0; i < amble_errs.size(); i++){
            printf("%d: %d errors.\n", i, amble_errs.at(i));
            printf("%s\n", std::bitset<6>((out8[0] & amblemask[0]) >> 2).to_string().c_str());
            printf("%s\n", std::bitset<6>(
                (extractBitSequence(out8, 4, i) & amblemask[0]) >> 2
                ).to_string().c_str());
        }
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