#include <iostream>
#include "timer.h"
#include <vector>

template <typename T>
void naiveAdd(const std::vector<T> &x ,const std::vector<T> &y, std::vector<T> &z)
{
    for (int i = 0; i < x.size(); ++i)
        z[i] = x[i] + y[i];
}

// note: we do this because msvc is terrible at autovectorising the naiveAdd function
// it will complain with an error code and not vectorise it at all until we use pointers
// gcc and clang don't do this! what is msvc doing...
template <typename T>
void naiveAddPointers(const T* x, const T* y, T *z, int size)
{
    for (int i = 0; i < size; ++i)
        z[i] = x[i] + y[i];
}

template <typename T>
void explicitUnrollAdd(const std::vector<T> &x ,const std::vector<T> &y, std::vector<T> &z)
{
    for (int i = 0; i < x.size(); i += 5)
    {
         z[i] = x[i] + y[i];
         z[i+1] = x[i+1] + y[i+1];
         z[i+2] = x[i+2] + y[i+2];
         z[i+3] = x[i+3] + y[i+3];
         z[i+4] = x[i+4] + y[i+4];
    }
}

         
int main()
{
    size_t len = 1000000;
    int numLoops = 10;

    std::vector<float> x(len);
    std::vector<float> y(len);
    std::vector<float> z(len);

    {
        HighResolutionTimer timer;
        for (int i = 0; i < numLoops; ++i)
            naiveAdd(x,y,z);
    }

    {
        HighResolutionTimer timer;
        for (int i = 0; i < numLoops; ++i)
            naiveAddPointers(x.data(), y.data(), z.data(), x.size()); 
    }


    {
        HighResolutionTimer timer;
        for (int i = 0; i < numLoops; ++i)
            explicitUnrollAdd(x,y,z);
    }


    return 0;
}
