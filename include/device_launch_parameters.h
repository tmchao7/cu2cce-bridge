#ifndef __CU2CCE_DEVICE_LAUNCH_PARAMETERS_H__
#define __CU2CCE_DEVICE_LAUNCH_PARAMETERS_H__

/*
 * NVIDIA-parity stub: threadIdx/blockIdx/blockDim/gridDim are compiler
 * builtins under ccec --enable-simt (existing samples use them without any
 * extra include). This header exists so upstream sources may include it,
 * and pulls in the vector shapes some of them rely on afterwards.
 */

#include <crt/vector_types.h>

#endif /* __CU2CCE_DEVICE_LAUNCH_PARAMETERS_H__ */
