#include <cuda_runtime.h>
#include <stdio.h>

__global__ static void write_value(int* out) {
  if (threadIdx.x == 0 && blockIdx.x == 0)
    *out = 42;
}

static int check_cuda(cudaError_t err, const char* step) {
  if (err == cudaSuccess)
    return 1;

  fprintf(stderr, "%s failed: %s\n", step, cudaGetErrorString(err));
  return 0;
}

int main(void) {
  int* device_value = NULL;
  int host_value = 0;

  if (!check_cuda(cudaMalloc((void**)&device_value, sizeof(*device_value)), "cudaMalloc"))
    return 1;

  write_value<<<1, 1>>>(device_value);
  if (!check_cuda(cudaGetLastError(), "kernel launch")) {
    cudaFree(device_value);
    return 1;
  }

  if (!check_cuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize")) {
    cudaFree(device_value);
    return 1;
  }

  if (!check_cuda(cudaMemcpy(&host_value, device_value, sizeof(host_value), cudaMemcpyDeviceToHost), "cudaMemcpy")) {
    cudaFree(device_value);
    return 1;
  }

  if (!check_cuda(cudaFree(device_value), "cudaFree"))
    return 1;

  printf("CUDA value: %d\n", host_value);
  return 0;
}
