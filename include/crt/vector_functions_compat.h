#ifndef __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__
#define __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__

/*
 * Documentation-only again — chain-included headers must be INERT until
 * every symbol they reference is proven to exist on the target toolchain.
 *
 * Server evidence so far (2026-08-27):
 *   make_float3  : EXISTS, device-only  (asc/impl/simt_api/
 *                  vector_functions_impl.h, __SIMT_DEVICE_FUNCTIONS_DECL__)
 *   double2 type : DOES NOT EXIST ("unknown type name 'double2'" through
 *                  the whole cuda_runtime.h chain — therefore NO make_
 *                  entry may reference it)
 *
 * This empty version restores the required gates. Future factory entries
 * come back one-at-a-time, each behind evidence gathered by the extended
 * inventory step (grep of __clang_dpp_types.h + vector_functions_impl.h),
 * never speculatively. Host-callability templates for the covered subset
 * live in git history of this file.
 */

#endif /* __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__ */
