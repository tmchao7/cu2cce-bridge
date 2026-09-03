/* Compile-only public CUDA header parity smoke; no CANN library is linked. */
#include <cuda.h>
#include <driver_functions.h>
#include <cublas.h>
#include <cublasLt.h>
#include <cufft.h>
#include <curand.h>
#include <cusolverDn.h>
#include <cusparse.h>
#include <cudnn.h>
#include <nccl.h>
#include <npp.h>
#include <nvjpeg.h>
#include <nvml.h>
#include <nvrtc.h>
#include <cudaEGL.h>
#include <cudaGL.h>
#include <cudaVDPAU.h>

int main() { return 0; }
