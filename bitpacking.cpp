#include "timer.h"
#include <iostream>
#include <stdint.h>
#include <vector>
#include "immintrin.h"

void naivePack(const uint8_t *unpacked, uint8_t *packed, int length)
{
    uint8_t tmp;

    for (int i = 0; i < length/8; i++)
    {
        // Pick each bit
        tmp = (unpacked[i*8] << 7) | (unpacked[i*8+1] << 6) | (unpacked[i*8+2] << 5) | (unpacked[i*8+3] << 4) | (unpacked[i*8+4] << 3) | (unpacked[i*8+5] << 2) | (unpacked[i*8+6] << 1) | (unpacked[i*8+7] << 0);
        packed[i] = tmp;
    } // Validated logic
}

void intrinsicPack(const uint8_t *unpacked, uint8_t *packed, int length, bool reverse=true)
{
    uint64_t *ptr;
    for (int i = 0; i < length/8; i++)
    {
        ptr = (uint64_t*)&unpacked[i*8];
        packed[i] = _pext_u64(*ptr, 0x0101010101010101ULL);
    }
    // Validated logic, but is reverse of the naive one
    if (reverse)
    {
        for (int i = 0; i < length/8; i++)
        {
            packed[i] = (packed[i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;
        }       
    }
}


int main()
{
    int length = 10000000;
    std::vector<uint8_t> unpacked(length);
    std::vector<uint8_t> packed(length/8);

    for (int i = 0; i < length; i++)
    {
        unpacked.at(i) = i % 2;
    }

    // Call naive
    {
        HighResolutionTimer timer;
        naivePack(unpacked.data(), packed.data(), length);
    }

    // Print the first of each
    for (int i = 0; i < 8; i++)
    {
        printf("%d", unpacked.at(i));
    }
    printf("\n");
    printf("%d\n", packed.at(0));

    // Call intrinsic
    {
        HighResolutionTimer timer;
        intrinsicPack(unpacked.data(), packed.data(), length);
        // With reverse added (to make it equivalent), the timing is 0.0065s vs 0.0075s, a marginal speedup.
        // Without reverse, it is about 2x speed.
    }

    // Print the first of each
    for (int i = 0; i < 8; i++)
    {
        printf("%d", unpacked.at(i));
    }
    printf("\n");
    printf("%d\n", packed.at(0));



    return 0;
}