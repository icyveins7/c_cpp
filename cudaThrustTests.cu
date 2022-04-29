#include "cuda.h"
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "npp.h" // interoperable with NPP types?
#include "thrust/system/cuda/experimental/pinned_allocator.h" // need this specifically

// namespace for experimental allocator is too long
namespace cuta = thrust::system::cuda::experimental;

#define LENGTH 1000000

int main(int argc, char *argv[])
{
    // Attempt to allocate a simple 32f host vector
    thrust::host_vector<float, cuta::pinned_allocator<float>> hv_32f(LENGTH);

    // And a similar device vector
    thrust::device_vector<float> dv_32f(LENGTH);

    // Attempt to copy over
    dv_32f = hv_32f;

    return 0;
}