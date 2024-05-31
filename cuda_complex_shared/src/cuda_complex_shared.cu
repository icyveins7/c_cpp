#include "cuComplex.h"
#include "cuda.h"

/*
This is the simple method of storing complex arrays into shared mem.
It simply packs everything in as-is.
*/
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
  z[idx] = s_x[idx] + s_y[idx];
}

/*
clang-format off

In this one we add a stagger between the two arrays in shared memory.
Assuming a blockDim of multiples of 32, we would have

  0| 1| 2|...................|30|31
 re|im|re|im.................|re|im
.......................
.......................
NUL|re|im| ................        -> this line is the start of y, which is offset by 1 bank

In hindsight, I think this doesn't matter because during the writes are
separate, and the reads are also separate

clang-format on
*/
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
  z[idx] = s_x[idx] + s_y[idx];
}

/*
In this one we split the data into separate real/complex arrays.

Consider the stores from global mem -> shared mem.
Under the ordinary packed format, the warp would request for 32*8=256 bytes,
which would be serviced in 2 memory transactions.

This would essentially store indices 0-15, then 16-31, in 2 consecutive rows.

During the addition step, two separate things must happen.
1) The request from the warp is again split into 2 transactions.
2) The addition instruction is then executed for reals,
3) and then for imaginary numbers.

What is unclear is whether the addition instructions are executed after each
transaction, or they are amalgamated after both transactions.

In the former, 4 addition instructions would be needed (1 for each memory
transaction, and for real/imag).

In the latter, only 2 addition instruction would be needed?

Without some deeper investigation into the assembly, it may be difficult to
discern what would happen.



Instead, this kernel splits the reals and imaginaries into two separate arrays
after reading from global mem in a contiguous manner.

There would still be 2 memory transactions, 1 for the reals and 1 for the imags.

During the addition, it would be obvious that the warp would be able to
access all 32 reals without bank conflicts, and similarly for all 32 imags.

Then 2 addition instructions are executed, 1 for the reals and 1 for the imags.

So in this case it feels more explicit(?) that the instruction count is lower.
*/
__global__ void cplxAddDeinterleavedShared(const cuComplex *x,
                                           const cuComplex *y, cuComplex *z,
                                           int n) {

  int idx = threadIdx.x + blockDim.x * blockIdx.x;

  // assign shared memory things
  extern __shared__ double s[];

  // allocate each workspace to be same as blockDim
  // leave a space of 32-bits after the first one
  float *s_x_r = (float *)s; // we use float here to count elements
  float *s_x_i = (float *)&s_x_r[blockDim.x];
  float *s_y_r = (float *)&s_x_i[blockDim.x];
  float *s_y_i = (float *)&s_y_r[blockDim.x];

  // Read to thread-local contiguously first
  float s_x = x[idx];

  // Then split
  s_x_r[threadIdx.x] = s_x.real();
  s_x_i[threadIdx.x] = s_x.imag();

  // Similar for y
  float s_y = y[idx];

  s_y_r[threadIdx.x] = s_y.real();
  s_y_i[threadIdx.x] = s_y.imag();

  __syncthreads();

  // add locally first and then write out
  float z_r = s_x_r[threadIdx.x] + s_y_r[threadIdx.x];
  float z_i = s_x_i[threadIdx.x] + s_y_i[threadIdx.x];

  z[idx] = cuComplex(z_r, z_i);
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
