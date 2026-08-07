#ifndef __CUDA_WRAPPER_TYPES_H__
#define __CUDA_WRAPPER_TYPES_H__

/*
 * cu2cce-bridge 自定义类型。
 *
 * 仿照 cu-bridge：wrapper 声明用 mc* 类型，宏表把 cuda* 替换为 mc*。
 * cu2cce 的目标 API 是昇腾 acl*，但 `cudaDeviceProp` 这类"结构化描述"没有一一对应的
 * acl 结构（aclrtDeviceProperties 的字段名与 cudaDeviceProp 不同），
 * 因此由本头定义一个 _cceDeviceProp_t 模拟 cudaDeviceProp 常用字段，
 * 由 wcudaGetDeviceProperties 从 aclrtGetDeviceProperties 填充。
 *
 * 字段集合刻意只覆盖 CUDA 应用最常用的子集（vectorAdd / matrixMul 等 sample 够用），
 * 需要更多字段时按 cuda_runtime_api.h 的 cudaDeviceProp 定义补充即可。
 */

#ifdef __cplusplus
extern "C" {
#endif

#define _CCE_DEVICE_PROP_MAX_NAME_LEN 256

typedef struct _cceDeviceProp_t {
    char name[_CCE_DEVICE_PROP_MAX_NAME_LEN];

    /* 计算能力（昇腾无 CUDA 语义的计算能力，此处填 0 或按 SOC 型号映射，应用一般不用） */
    int major;
    int minor;

    size_t totalGlobalMem; /**< 可用显存总量（Bytes），从 aclrtGetMemInfo 查询 */
    size_t sharedMemPerBlock; /**< 每 block 共享内存上限（昇腾对应 UB 容量，见 SIMT 256KB） */
    int regsPerMultiprocessor; /**< 预留 */
    int warpSize; /**< SIMT 下 warp=32；传统 tile 模式无意义，填 32 兼容应用 */
    size_t memPitch;
    int maxThreadsPerBlock; /**< SIMT 下 block 内线程上限 2048 */
    int maxThreadsDim[3];
    int maxGridSize[3];
    int clockRate;
    size_t totalConstMem;
    int multiProcessorCount; /**< 昇腾对应 AIV 核数（blockDim 上限），从 aclrtGetDeviceProperties 查询 */
    int l2CacheSize;
    int maxThreadsPerMultiProcessor;
    int computeMode;
} _cceDeviceProp_t;

#ifdef __cplusplus
}
#endif

#endif //__CUDA_WRAPPER_TYPES_H__
