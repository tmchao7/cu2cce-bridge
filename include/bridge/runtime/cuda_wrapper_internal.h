#ifndef __CUDA_WRAPPER_INTERNAL_H__
#define __CUDA_WRAPPER_INTERNAL_H__

/*
 * 内部符号声明。仿照 cu-bridge 的 cuda_wrapper_internal.h。
 *
 * 职责：
 *  - 提供 bridge 初始化入口（bridge_init，用 std::call_once 保证只跑一次）。
 *  - 提供版本号内部符号 __wcuda_version_internal__（对应 cu-bridge 的 __wcuda_version_internal__）。
 *
 * 昇腾侧 bridge_init 需要做的事（真机上实现）：
 *  - 加载 CANN 运行库（libacl_rt.so）——CANN 头文件默认在 ${ASCEND_TOOLKIT_HOME}/include，
 *    库在 ${ASCEND_TOOLKIT_HOME}/lib64；链接期由 bin/cucc 负责 -L/-l，运行期由 loader 找。
 *  - 必要时预初始化 ACL（但注意：CUDA 的初始化是"首次调用时惰性"的，
 *    aclInit 也应在第一次 wcuda* 调用时由 WCUDA_RETURN 宏触发，而非在此处立即执行）。
 */
#include <cstdio>
#include <mutex>

#include "cuda_wrapper_internal_types.h"

extern CUBRIDGE_API int __wcuda_version_internal__;

namespace cubridge::internal {

static std::once_flag bridge_init_flag_;

void bridge_init();

} // namespace cubridge::internal

/* bridge 初始化：第一次任何 wcuda* 调用时执行一次。
 * 仿照 cu-bridge 的 BRIDGE_INIT 宏（见 cuda_driver_wrapper.h）。 */
#define BRIDGE_INIT()                                                                              \
    do {                                                                                           \
        std::call_once(cubridge::internal::bridge_init_flag_, cubridge::internal::bridge_init);    \
    } while (0);

#endif //__CUDA_WRAPPER_INTERNAL_H__
