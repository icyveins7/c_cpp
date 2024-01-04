// This is an explicit test to compare our own iterative Tone implementation against IPP's one.
// The stackoverflow article is at https://stackoverflow.com/questions/51735576/fast-and-accurate-iterative-generation-of-sine-and-cosine-for-equally-spaced-ang

#include <iostream>
#include <vector>
#include <complex>

int main()
{
    double startPhase = 0.0;
    double rFreq = 0.0001;
    size_t length = 100000;

    // First value
    std::complex<double> tone(std::cos(startPhase), std::sin(startPhase));
    // Step
    std::complex<double> step(std::cos(rFreq), std::sin(rFreq));

    for (size_t i = 1; i < length; ++i)
    {
        tone *= step;
    }

    printf("Tone after %zd steps is (%f, %f)\n", length, tone.real(), tone.imag());



    return 0;
}