#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <boost/functional/hash.hpp>
#include "timer.h"
#include <cstdlib>
#include <ctime>

struct pair_hash{
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.first);
        boost::hash_combine(seed, p.second);

        return seed;
    }
};

int main()
{
    std::srand(std::time(nullptr));

    const int N = 1000000;

    std::unordered_map<std::pair<float, float>, float, pair_hash> m;
    std::cout << "unordered_map with boost::hash_combine" << std::endl;
    {
        HighResolutionTimer timer;
        for (int i = 0; i < N; ++i)
        {
            std::pair<float, float> key{
                (float)std::rand(),
                (float)std::rand()};
            float val = (float)(i+3);
            m.insert({key, val});
        }
    }


    std::unordered_map<std::pair<float, float>, float, pair_hash> mm(N);
    std::cout << "unordered_map with boost::hash_combine and min N buckets" << std::endl;
    {
        HighResolutionTimer timer;
        for (int i = 0; i < N; ++i)
        {
            std::pair<float, float> key{
                (float)std::rand(),
                (float)std::rand()};
            float val = (float)(i+3);
            mm.insert({key, val});
        }
    }


    std::map<std::pair<float, float>, float> m2;
    std::cout << "map" << std::endl;
    {
        HighResolutionTimer timer;
        for (int i = 0; i < N; ++i)
        {
            std::pair<float, float> key{
                (float)std::rand(),
                (float)std::rand()};
            float val = (float)(i+3);
            m2.insert({key, val});
        }
    }


    std::vector<float> v2(N);
    std::cout << "pure vector with reserved N, accessed via modulus" << std::endl;
    {
        HighResolutionTimer timer;
        for (int i = 0; i < N; ++i)
        {
            // simulate range of pixel estimation
            size_t idx = (std::rand() / 37 + 103) % N;
            v2.at(idx) = (float)(i+3);
        }
    }


    return 0;
}
