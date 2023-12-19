#include "timer.h"
#include <iostream>
#include <vector>
#include <random>
#include <stdint.h>

void argb_lut(
    const float min,
    const float range,
    const std::vector<uint32_t> &lut,
    const std::vector<float> &in,
    std::vector<uint32_t> &out
)
{
    for (int i = 0; i < out.size(); ++i)
    {
        int idx = (int)((in.at(i) - min) / range * (lut.size()-1));
        out.at(i) = lut.at(idx);
    }
}

int main()
{ 
    for (int loop = 0; loop < 10; ++loop)
    {
        // Fill a LUT
        std::vector<uint32_t> lut(256);
        for (auto &i : lut)
        {
            i = std::rand(); 
        }

        // Define some values
        float min = 0.0f;
        float range = 1.0f;
        size_t len = 100000000;
        std::vector<float> in(len);
        // Fill input
        for (auto &i : in)
        {
            i = std::rand() / (float)RAND_MAX;
        }
        // Declare output
        std::vector<uint32_t> out(len);


        // Time filling output from LUT
        {
            HighResolutionTimer timer;

            argb_lut(min, range, lut, in, out);
        }

        // Print some output
        for (int i = 0; i < 10; ++i)
        {
            int idx = std::rand() % out.size();
            printf("%d: %d\n", idx, out.at(idx));
        }
        
    }

    

    return 0;
}