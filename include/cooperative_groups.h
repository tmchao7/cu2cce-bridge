#ifndef __CU2CCE_COOPERATIVE_GROUPS_H__
#define __CU2CCE_COOPERATIVE_GROUPS_H__

/*
 * CUDA cooperative_groups compatibility shim for Ascend SIMT.
 *
 * CANN provides the actual group types and synchronization/shuffle primitives
 * in simt_api/cooperative_groups.h. Do not redeclare those types here: doing so
 * creates duplicate definitions when asc_simt.h brings the official CANN header
 * into the same translation unit. This shim only supplies the CUDA spelling
 * and the small reduce/plus surface used by the external reduction sample.
 */

#if defined(__has_include)
#  if __has_include(<simt_api/cooperative_groups.h>)
#    include <simt_api/cooperative_groups.h>
#  else
#    error "cu2cce: CANN simt_api/cooperative_groups.h is required"
#  endif
#else
#  include <simt_api/cooperative_groups.h>
#endif

/* CANN's public SIMT declarations use this compiler-level qualifier. */
#ifndef __SIMT_DEVICE_FUNCTIONS_DECL__
#  define __SIMT_DEVICE_FUNCTIONS_DECL__ __aicore__
#endif
#ifndef __device__
#  define __device__ __SIMT_DEVICE_FUNCTIONS_DECL__
#endif
#ifndef __forceinline__
#  define __forceinline__ inline
#endif

namespace cooperative_groups {

template <typename T>
struct plus {
    __SIMT_DEVICE_FUNCTIONS_DECL__ inline T operator()(const T &lhs, const T &rhs) const {
        return lhs + rhs;
    }
};

template <typename Group, typename T, typename BinaryOp>
__SIMT_DEVICE_FUNCTIONS_DECL__ inline T reduce(Group &group, T value, BinaryOp op) {
    for (unsigned int offset = group.size() / 2; offset > 0; offset /= 2) {
        value = op(value, group.shfl_down(value, offset));
    }
    return value;
}

} // namespace cooperative_groups

#endif // __CU2CCE_COOPERATIVE_GROUPS_H__
