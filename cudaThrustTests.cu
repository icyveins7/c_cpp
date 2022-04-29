/*
If kernels are not running, run deviceQuery and add flag -arch=sm_50 <- or whatever your device cuda capability is
*/

#include "cuda.h"
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "npp.h" // interoperable with NPP types?
#include "thrust/system/cuda/experimental/pinned_allocator.h" // need this specifically

// namespace for experimental allocator is too long
namespace tsce = thrust::system::cuda::experimental;

#define LENGTH 1000000

__global__
void myadd_kernel(float* v, int len)
{
    int tIdx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = gridDim.x * blockDim.x;

    for (int i = tIdx; i < len; i = i + stride)
    {
        v[i] = (float)i;
    }
}

int main(int argc, char *argv[])
{
    // Attempt to allocate a simple 32f host vector (pinned + unpinned)
    thrust::host_vector<float, tsce::pinned_allocator<float>> hv_32f(LENGTH);
    thrust::host_vector<float> hv_32f_unpin(LENGTH);

    // And a similar device vector
    thrust::device_vector<float> dv_32f(LENGTH);

    // Raw allocation
    float* darr_32f, *harr_32f;
    cudaMalloc((void**)&darr_32f, sizeof(float) * LENGTH);
    cudaMallocHost((void**)&harr_32f, sizeof(float) * LENGTH);

    // Attempt to copy over
    dv_32f = hv_32f_unpin; // note that this still causes async copies? how is that possible?
    dv_32f = hv_32f; // async copy, as viewed in nvvp

    // via explicit 'copy'
    thrust::copy(hv_32f_unpin.begin(), hv_32f_unpin.end(), dv_32f.begin()); // this is still async copy?
    // however, all async copies in the default stream are serialized anyway..

    // Write using a kernel?
    myadd_kernel << <2, 512 >> > ((float*)thrust::raw_pointer_cast(dv_32f.data()), dv_32f.size());
    myadd_kernel << < 2, 512 >> > (darr_32f, LENGTH);

    // Copy it back
    hv_32f = dv_32f;

    for (int i = 0; i < 5; i++)
    {
        printf("%d: %f\n", i, hv_32f[i]);
    }

    // Raw copy back
    cudaMemcpy(harr_32f, darr_32f, sizeof(float) * LENGTH, cudaMemcpyDeviceToHost);
    for (int i = 0; i < 5; i++)
    {
        printf("%d: %f\n", i, harr_32f[i]);
    }

    // cleanup
    cudaFree(darr_32f);
    cudaFreeHost(harr_32f);

    return 0;
}