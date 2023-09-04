#include <iostream>
#include <vector>

void thread_to_index_contiguous_split(size_t tidx, size_t size, size_t numThreads, size_t &start, size_t &end)
{
    size_t perThread = size / numThreads;
    size_t remainder = size % numThreads;

    // These threads need to do 1 more
    if (tidx < remainder)
    {
        start = (perThread + 1) * tidx;
        end = start + perThread + 1;
    }
    else // the rest do the smaller number
    {
        start = remainder * (perThread + 1) + (tidx - remainder) * perThread;
        end = start + perThread;
    }
}

int main(int argc, char **argv)
{
    std::vector<int> x(25);
    size_t numThreads = 6;

    size_t start, end;
    for (size_t tidx = 0; tidx < numThreads; tidx++)
    {
        thread_to_index_contiguous_split(tidx, x.size(), numThreads, start, end);
        printf("Thread %d starts at %zu and ends at %zu, total of %zd\n", tidx, start, end, end-start);
    }


    return 0;
}