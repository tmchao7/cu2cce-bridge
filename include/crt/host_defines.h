#ifndef __CU2CCE_CRT_HOST_DEFINES_H__
#define __CU2CCE_CRT_HOST_DEFINES_H__

/*
 * Minimal CUDA crt macro surface (parity with NVIDIA crt/host_defines.h).
 *
 * ccec --enable-simt already understands and ENFORCES __host__/__device__
 * execution-space semantics (real server diagnostics: "candidate function
 * not viable: call to __device__ function from __host__ function"), so the
 * spellings below are only declared for toolchains that lack them. No
 * execution-space meaning is added or removed here.
 */

#if !defined(__host__)
#define __host__
#endif

#if !defined(__device__)
#define __device__
#endif

#if !defined(__forceinline__)
#define __forceinline__ inline
#endif

/* CUDA launches expose grid/block geometry through compiler builtins; this
 * header only mirrors NVIDIA's typedef surface for host-side structures. */

#endif /* __CU2CCE_CRT_HOST_DEFINES_H__ */
