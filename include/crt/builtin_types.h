#ifndef __CU2CCE_CRT_BUILTIN_TYPES_H__
#define __CU2CCE_CRT_BUILTIN_TYPES_H__

/*
 * Parity with NVIDIA crt/builtin_types.h: the vector shape surface. The
 * ushort/uint/ulong typedef family is deliberately NOT declared here —
 * system headers commonly define these macros, and silently redefining
 * them would poison large host projects. Add aliases only with real
 * evidence that a consumer needs them.
 */

#include <crt/vector_types.h>

#endif /* __CU2CCE_CRT_BUILTIN_TYPES_H__ */
