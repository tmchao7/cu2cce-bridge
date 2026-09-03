#ifndef __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_GL_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_GL_WRAPPER_H__

/* CUDA compatibility header parity entry. */
/* Reference header: include/bridge/runtime/cuda_gl_wrapper.h */
/* This is a cu2cce compatibility surface, not an NVIDIA runtime copy. */
/* Status: RESERVED_NPU or DIAGNOSTIC_ONLY until each API has a safe CANN ABI. */
/* API names and future routes are recorded below; no fake library is linked. */
/* CUDA API: cuGLCtxCreate
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLGetDevices
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLInit
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLMapBufferObject
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLMapBufferObjectAsync
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLMapBufferObjectAsync_v2
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLMapBufferObject_v2
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLRegisterBufferObject
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLSetBufferObjectMapFlags
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLUnmapBufferObject
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLUnmapBufferObjectAsync
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGLUnregisterBufferObject
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGraphicsGLRegisterBuffer
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGraphicsGLRegisterImage
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuWGLGetDevice
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLGetDevices
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLMapBufferObject
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLMapBufferObjectAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLRegisterBufferObject
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLSetBufferObjectMapFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLSetGLDevice
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLUnmapBufferObject
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLUnmapBufferObjectAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGLUnregisterBufferObject
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphicsGLRegisterBuffer
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphicsGLRegisterImage
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaWGLGetDevice
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */

#endif /* __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_GL_WRAPPER_H__ */
