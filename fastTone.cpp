// This is an explicit test to compare our own iterative Tone implementation against IPP's one.
// The stackoverflow article is at https://stackoverflow.com/questions/51735576/fast-and-accurate-iterative-generation-of-sine-and-cosine-for-equally-spaced-ang

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <complex>
#include "ipp_ext.h"
#include "timer.h"

template<typename T>
std::complex<T> iterTone(T startPhase, T rFreq, size_t length)
{
    std::complex<T> tone(std::cos(startPhase), std::sin(startPhase));
    std::complex<T> step(std::cos(2*M_PI*rFreq), std::sin(2*M_PI*rFreq));

    for (size_t i = 1; i < length; ++i)
    {
        tone *= step;
    }

    return tone;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::complex<T> iterToneUnroll2(T startPhase, T rFreq, size_t length)
{
    std::complex<T> tone1(std::cos(startPhase), std::sin(startPhase));
    std::complex<T> tone2(std::cos(2*M_PI*rFreq + startPhase), std::sin(2*M_PI*rFreq + startPhase));
    std::complex<T> step(std::cos(2*M_PI*rFreq*2), std::sin(2*M_PI*rFreq*2));

    for (size_t i = 2; i < length; i += 2)
    {
        tone1 *= step;
        tone2 *= step;
    }

    return tone1+tone2; // this is incorrect but done to ensure that the computation isn't optimised away
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::complex<T> iterToneUnroll4(T startPhase, T rFreq, size_t length)
{
    std::complex<T> tones[4];
    // Init the first few
    for (int i = 0; i < 4; ++i)
        tones[i] = std::complex<T>(std::cos(startPhase + i*2*M_PI*rFreq), std::sin(startPhase + i*2*M_PI*rFreq));

    std::complex<T> step(std::cos(2*M_PI*rFreq*4), std::sin(2*M_PI*rFreq*4));


    for (size_t i = 4; i < length; i += 4)
    {
        tones[0] *= step;
        tones[1] *= step;
        tones[2] *= step;
        tones[3] *= step;
    }

    // return tones[0] + tones[1] + tones[2] + tones[3]; // this is incorrect but done to ensure that the computation isn't optimised away
    return tones[3];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::complex<T> iterToneUnroll8(T startPhase, T rFreq, size_t length)
{
    std::complex<T> tones[8];
    // Init the first few
    for (int i = 0; i < 8; ++i)
        tones[i] = std::complex<T>(std::cos(startPhase + i*2*M_PI*rFreq), std::sin(startPhase + i*2*M_PI*rFreq));

    std::complex<T> step(std::cos(2*M_PI*rFreq*8), std::sin(2*M_PI*rFreq*8));


    for (size_t i = 8; i < length; i += 8)
    {
        tones[0] *= step;
        tones[1] *= step;
        tones[2] *= step;
        tones[3] *= step;
        tones[4] *= step;
        tones[5] *= step;
        tones[6] *= step;
        tones[7] *= step;
    }

    return tones[0] + tones[1] + tones[2] + tones[3] + tones[4] + tones[5] + tones[6] + tones[7]; // this is incorrect but done to ensure that the computation isn't optimised away
}
// Seems like 4 unrolls is good enough, marginal improvements when going to 8

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t UNROLL>
std::complex<T> iterToneUnrollN(T startPhase, T rFreq, size_t length)
{
    std::complex<T> tones[UNROLL];
    // Init the first few
    for (size_t i = 0; i < UNROLL; ++i)
        tones[i] = std::complex<T>(std::cos(startPhase + i*2*M_PI*rFreq), std::sin(startPhase + i*2*M_PI*rFreq));

    std::complex<T> step(std::cos(2*M_PI*rFreq*UNROLL), std::sin(2*M_PI*rFreq*UNROLL));

    for (size_t i = UNROLL; i < length; i += UNROLL)
    {
        for (size_t j = 0; j < UNROLL; ++j)
        {
            tones[j] *= step;
        }
    }

    // // Sum the tones so that it isn't optimised away
    // for (size_t i = 0; i < UNROLL-1; ++i)
    //     tones[UNROLL-1] += tones[i];

    return tones[UNROLL-1];
}

int main()
{
    double startPhase = 0.0;
    double rFreq = 1e-9;
    size_t length = 10000000;
    int repeats = 5;

    std::complex<double> exact(
        std::cos(startPhase + 2*M_PI*rFreq),
        std::sin(startPhase + 2*M_PI*rFreq)
    );
    printf("Exact value is (%f, %f)\n", exact.real(), exact.imag());

    constexpr size_t UNROLL = 4;

    {
        HighResolutionTimer timer;

        std::complex<double> tone;
        for (int i = 0; i < repeats; ++i)
        {
            tone = iterToneUnroll4<double>(startPhase, rFreq, length);
            // tone = iterToneUnrollN<double, UNROLL>(startPhase, rFreq, length);

        }
        printf("Tone (%zd unrolls) after %zd steps is (%f, %f)\n", UNROLL, length, tone.real(), tone.imag());
    }
    

    

    {
        HighResolutionTimer timer;
        std::complex<float> toneFloat;
        for (int i = 0; i < repeats; ++i)
        {
            toneFloat = iterToneUnroll4<float>(
                static_cast<float>(startPhase), 
                static_cast<float>(rFreq), 
                length);

            // toneFloat = iterToneUnrollN<float, UNROLL>(
            //     static_cast<float>(startPhase), 
            //     static_cast<float>(rFreq), 
            //     length);
        }
        printf("ToneFloat (%zd unrolls) after %zd steps is (%f, %f)\n", UNROLL, length, toneFloat.real(), toneFloat.imag());
    }
    


    ippe::vector<Ipp64fc> iTone((int)length);

    // IPP first call has some initial loading time
    {
        HighResolutionTimer timer;
        for (int i = 0; i < repeats; ++i)
        {
            double phase = startPhase;
            ippe::generator::Tone(iTone.data(), (int)iTone.size(), 1.0, rFreq, &phase, IppHintAlgorithm::ippAlgHintFast);
            
        }
        printf("ippsTone (Fast) last value is (%f, %f)\n", iTone.back().re, iTone.back().im);
    }

    {
        HighResolutionTimer timer;
        for (int i = 0; i < repeats; ++i)
        {
            double phase = startPhase;
            ippe::generator::Tone(iTone.data(), (int)iTone.size(), 1.0, rFreq, &phase, IppHintAlgorithm::ippAlgHintAccurate);
            
        }
        printf("ippsTone (Accurate) last value is (%f, %f)\n", iTone.back().re, iTone.back().im);
    }
    
    
    



    return 0;
}