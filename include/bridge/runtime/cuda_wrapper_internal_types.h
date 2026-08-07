#ifndef __CUDA_WRAPPER_INTERNAL_TYPES_H__
#define __CUDA_WRAPPER_INTERNAL_TYPES_H__

/*
 * 内部类型定义。仿照 cu-bridge 的 cuda_wrapper_internal_types.h。
 *
 * 昇腾侧没有"NV arch 模拟"需求（cu-bridge 用它来把 mc 的 major/minor 伪装成
 * CUDA 计算能力），故此处仅保留未来可能用到的扩展位。
 */

/* 预留：昇腾 SOC 型号枚举（未来可在 cucc 编译期通过 -D 注入，用于 kernel/驱动分支）。
 * 示例值：ASCEND_910B / ASCEND_910C / ASCEND_950PR / ASCEND_950DT。 */
enum wcceSocType {
    __OTHER_SOC = 0,
    __SOC_ASCEND_910B,
    __SOC_ASCEND_910C,
    __SOC_ASCEND_950PR,
    __SOC_ASCEND_950DT,
};

#endif //__CUDA_WRAPPER_INTERNAL_TYPES_H__
