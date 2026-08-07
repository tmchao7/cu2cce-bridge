/*
 * cublas_v2.h —— cublas 兼容入口（Phase A4 最小子集）。
 *
 * 仿照 cu-bridge 的 include/cublas_v2.h：用户代码 #include <cublas_v2.h>
 * 落入此文件，转到底层 bridge/blas/cublas_v2_wrapper.h。
 *
 * 当前只覆盖最小可用子集：Create/Destroy/SetStream/GetStream + Sgemm/Dgemm。
 * 其余 cublas 函数（Sgemv/Saxpy/矩阵类型等）随迭代补充，昇腾无对应的标注"不支持"。
 */
#if !defined(CUBLAS_V2_H_)
#define CUBLAS_V2_H_

#if defined(CUBLAS_H_)
#error "It is an error to include both cublas.h and cublas_v2.h"
#endif

#include <bridge/blas/cublas_v2_wrapper.h>

#endif /* !defined(CUBLAS_V2_H_) */
