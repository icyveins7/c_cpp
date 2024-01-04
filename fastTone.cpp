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
    std::complex<double> step(2*M_PI*std::cos(rFreq), 2*M_PI*std::sin(rFreq));

    for (size_t i = 1; i < length; ++i)
    {
        tone *= step;
    }

    printf("Tone after %zd steps is (%f, %f)\n", length, tone.real(), tone.imag());

    std::complex<double> exact(
        std::cos(startPhase + 2*M_PI*rFreq),
        std::sin(startPhase + 2*M_PI*rFreq)
    );
    printf("Exact value is (%f, %f)\n", exact.real(), exact.imag());



    return 0;
}