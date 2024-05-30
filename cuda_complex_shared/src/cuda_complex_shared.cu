#include "cuComplex.h"
#include "cuda.h"

__global__ void cplxAddPackedShared(const cuComplex *x, const cuComplex *y,
                                    cuComplex *z, int n) {

  int idx = threadIdx.x + blockDim.x * blockIdx.x;

  // assign shared memory things
  extern __shared__ double s[];

  // allocate each workspace to be same as blockDim
  // align one after another, i.e. well packed
  cuComplex *s_x = (cuComplex *)s;
  cuComplex *s_y = (cuComplex *)&s_x[blockDim.x];

  s_x[threadIdx.x] = x[idx];
  s_y[threadIdx.x] = y[idx];
  __syncthreads();

  // add and then write out
  z[idx] = s_x[idx] * s_y[idx];
}

__global__ void cplxAddStaggeredShared(const cuComplex *x, const cuComplex *y,
                                       cuComplex *z, int n) {

  int idx = threadIdx.x + blockDim.x * blockIdx.x;

  // assign shared memory things
  extern __shared__ double s[];

  // allocate each workspace to be same as blockDim
  // leave a space of 32-bits after the first one
  float *f_x = (float *)s; // we use float here to count elements
  float *f_gap = (float *)&f_x[2 * blockDim.x];
  float *f_y = (float *)&f_gap[1];

  // Cast to appropriate types
  cuComplex *s_x = (cuComplex *)f_x;
  cuComplex *s_y = (cuComplex *)f_y;

  s_x[threadIdx.x] = x[idx];
  s_y[threadIdx.x] = y[idx];
  __syncthreads();

  // add and then write out
  z[idx] = s_x[idx] * s_y[idx];
}

int main() {
  constexpr int LENGTH = 1000000;

  // Raw allocation
  cuComplex *d_x, *d_y, *d_z, *h_x, *h_y, *h_z;
  cudaMalloc((void **)&d_x, sizeof(cuComplex) * LENGTH);
  cudaMalloc((void **)&d_y, sizeof(cuComplex) * LENGTH);
  cudaMalloc((void **)&d_z, sizeof(cuComplex) * LENGTH);
  cudaMallocHost((void **)&h_x, sizeof(cuComplex) * LENGTH);
  cudaMallocHost((void **)&h_y, sizeof(cuComplex) * LENGTH);
  cudaMallocHost((void **)&h_z, sizeof(cuComplex) * LENGTH);

  // Raw copy
  cudaMemcpy(d_x, h_x, sizeof(cuComplex) * LENGTH, cudaMemcpyHostToDevice);
  cudaMemcpy(d_y, h_y, sizeof(cuComplex) * LENGTH, cudaMemcpyHostToDevice);

  // kernels
  constexpr int THREADS_PER_BLK = 128;
  const int numBlks = LENGTH / THREADS_PER_BLK;
  cplxAddPackedShared<<<numBlks, THREADS_PER_BLK, >>>(d_x, d_y, d_z, LENGTH);
  cplxAddStaggeredShared<<<numBlks, THREADS_PER_BLK, >>>(d_x, d_y, d_z, LENGTH);

  // Raw copy back
  cudaMemcpy(h_z, d_z, sizeof(cuComplex) * LENGTH, cudaMemcpyDeviceToHost);

  // cleanup
  cudaFree(d_x);
  cudaFree(d_y);
  cudaFree(d_z);
  cudaFreeHost(h_x);
  cudaFreeHost(h_y);
  cudaFreeHost(h_z);

  return 0;
}
