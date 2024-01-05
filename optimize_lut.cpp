
#define FP_FAST_FMAF
#include <cmath>
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
    const float multiplier = (lut.size() - 1.0f) / range;
    const float c = - min * (lut.size() - 1.0f) / range;

    for (int i = 0; i < out.size(); ++i)
    {
        int idx = (int)((in.at(i) - min) / range * (lut.size()-1));
        // int idx = (int)(
        //     std::fmaf(in.at(i), multiplier, c)
        // ); // slower than compiler's optimizations!
        out.at(i) = lut.at(idx);
    }
}

void argb_poly4(
    const float min,
    const float range,
    const std::vector<float> &in,
    std::vector<uint32_t> &out
){
    // assume a polynomial
    const float poly[4] = {5.52395531e-06, -2.09596383e-03,  2.45358561e-02,  7.47756316e+01};

    for (int i = 0; i < out.size(); ++i)
    {
        float scaled = (in.at(i) - min) / range;
        float col = poly[0];
        float powered = 1.0f;
        for (int j = 1; j < 4; ++j)
        {
            powered *= scaled;
            col += powered * poly[j];
        }
        out.at(i) = col;
    } // NOTE: this is not the same as we should be doing the polynomial for each component
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

        // Time filling output from polynomial
        {
            HighResolutionTimer timer;

            argb_poly4(min, range, in, out);
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