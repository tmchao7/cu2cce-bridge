#ifndef __CU2CCE_CUDA_LIBRARY_PLACEHOLDER_H__
#define __CU2CCE_CUDA_LIBRARY_PLACEHOLDER_H__

/*
 * Common placeholder for CUDA library headers whose Ascend/CANN ABI has not
 * been implemented by cu2cce-bridge. Including a header is source-compatible;
 * it does not provide an NVIDIA library, CUDA alias, or fake linker target.
 * The corresponding API names and future CANN route are recorded in
 * config/cuda_api_parity_inventory.json and bridge/cuda_to_cce_reserved_wrapper_parity.h.
 */
#define CU2CCE_CUDA_LIBRARY_PLACEHOLDER 1
#define CU2CCE_CUDA_LIBRARY_API_NOT_VERIFIED 1

#endif /* __CU2CCE_CUDA_LIBRARY_PLACEHOLDER_H__ */
