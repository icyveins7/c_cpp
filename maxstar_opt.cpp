#include "ipp_ext.h"
#include <cmath>
#include "timer.h"

/* Values for linear approximation (DecoderType=5) */
#define AJIAN -0.24904163195436
#define TJIAN 2.50681740420944

/* The linear-log-MAP algorithm */
float max_star0(
    float delta1, 
    float delta2 )
{
	register float diff;
	
	diff = delta2 - delta1;

	if ( diff > TJIAN )
		return( delta2 );
	else if ( diff < -TJIAN )
		return( delta1 );
	else if ( diff > 0 )
		return( delta2 + AJIAN*(diff-TJIAN) );
	else
		return( delta1 - AJIAN*(diff+TJIAN) );
}

float opt_maxstar(
    const float delta1,
    const float delta2 
){
    constexpr float tjian = 2.50681740420944f;
    constexpr float ajian = -0.24904163195436f;
    constexpr float ajian_m_tjian = ajian * -tjian;

    // Implementation 1 (faster than implementation 2 and 3)
    const float diff = delta2 - delta1;
    const float abs_diff = std::abs(diff);
    if (abs_diff > tjian)
    {
        if (diff > 0)
        {
            return delta2;
        }
        else
        {
            return delta1;
        }
    }
    else
    {
        float r = ajian*abs_diff + ajian_m_tjian;
        if (diff > 0)
        {
            return delta2 + r;
            // return delta2 + ajian*(diff-tjian);
        }
        else
        {
            return delta1 + r;
            // return delta1 - ajian*(diff+tjian);
        }
    }

    // // Implementation 3
    // float r = 0.0f;
    // const float diff = delta2 - delta1;
    // const float abs_diff = std::abs(diff);
    // const float fixed = ajian*abs_diff + ajian_m_tjian;
    // if (diff > 0)
    // {
    //     r = delta2;
    //     if (abs_diff <= tjian)
    //         r += fixed;
    // }
    // else
    // {
    //     r = delta1;
    //     if (abs_diff <= tjian)
    //         r += fixed;
    // }
    // return r;




    // // Implementation 2
    // const float diff = delta2 - delta1;
    // float r = (diff > 0) * delta2 + (diff <= 0) * delta1; // value to be returned
    // const float abs_diff = std::abs(diff);
    // if (abs_diff <= tjian)
    // {
    //     r += ajian_m_tjian + ajian * abs_diff;
    // }
    // return r;

}


int main()
{
    size_t length = 10000000;
    ippe::RandUniform<Ipp32f> rand(-10.0f, 10.0f);
    ippe::vector<Ipp32f> x(length);
    ippe::vector<Ipp32f> y(length);
    ippe::vector<Ipp32f> z1(length);
    ippe::vector<Ipp32f> z2(length);
    rand.generate(x.data(), (int)x.size());
    rand.generate(y.data(), (int)y.size());

    int loops = 10;


    // Time optimized call
    printf("Optimized:\n");
    {
        HighResolutionTimer timer;
        for (int l = 0; l < loops; l++)
        {
            for (int i = 0; i < x.size(); i++)
            {
                z2[i] = opt_maxstar(x[i], y[i]);
            } 
        }
    }


    // Time original call
    printf("Original:\n");
    {
        HighResolutionTimer timer;
        for (int l = 0; l < loops; l++)
        {
            for (int i = 0; i < x.size(); i++)
            {
                z1[i] = max_star0(x[i], y[i]);
            }
 
        }
   }

    // Check all equal
    for (int i = 0; i < z1.size(); i++)
    {
        if (z1[i] - z2[i] > 1e-5)
            printf("Error at index %d: %f vs %f\n", i, z1[i], z2[i]);
    }

    return 0;
}

