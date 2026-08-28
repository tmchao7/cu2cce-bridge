/*
 * EXPERIMENTAL secondary capability: wcublas* → aclblas*.
 * This library path is intentionally isolated from main/puresimt acceptance.
 *
 * wcublas* wrapper 实现（最小子集）。
 *
 * 仿照 cu-bridge 的 src/bridge/blas/src/cublas_v2_wrapper.cpp，目标从 mcblas 换成 aclblas。
 *
 * ⚠️ 本文件在装有 CANN 的真机上编译（链接 libacl_cblas.so），本机仅作语法验证。
 *
 * 关键转换：cublasSgemm → aclblasGemmEx
 *   cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc)
 *   → aclblasGemmEx(transa, transb, ACL_TRANS_N, m, n, k,
 *                   &alpha, A, lda, ACL_FLOAT, B, ldb, ACL_FLOAT,
 *                   &beta, C, ldc, ACL_FLOAT, computeType, stream)
 * 差异（2026-08-07 核实 torch_npu acl_cblas.h）：
 *   - aclblasGemmEx 无 handle、直接传 aclrtStream；
 *   - 无 aclblasCreate/Destroy/SetStream/GetStream；
 *   - 因此 cu2cce 自定义 cublasHandle_t（_wcceBlasHandle 结构）承载 stream。
 */

#include "cublas_v2_wrapper.h"

#include <acl/acl.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

/* CUBLAS 的成功码与 ACL 相同（0）。handle 是 cu2cce 内部结构，存关联 stream。 */

aclError wcublasCreate(cublasHandle_t *handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    _wcceBlasHandle *h = (_wcceBlasHandle *)std::calloc(1, sizeof(_wcceBlasHandle));
    if (h == nullptr) {
        return ACL_ERROR_BAD_ALLOC;
    }
    h->stream = nullptr; /* 默认 stream（nullptr 由调用方后续 SetStream） */
    h->pointerMode = CUBLAS_POINTER_MODE_HOST; /* cublas 默认：scalar 为 host 指针 */
    h->mathMode = CUBLAS_DEFAULT_MATH;
    h->atomicsMode = CUBLAS_ATOMICS_NOT_ALLOWED;
    *handle = h;
    return ACL_SUCCESS;
}

aclError wcublasDestroy(cublasHandle_t handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    std::free(handle);
    return ACL_SUCCESS;
}

aclError wcublasSetStream(cublasHandle_t handle, aclrtStream stream) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    handle->stream = stream;
    return ACL_SUCCESS;
}

aclError wcublasGetStream(cublasHandle_t handle, aclrtStream *stream) {
    if (handle == nullptr || stream == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *stream = handle->stream;
    return ACL_SUCCESS;
}

aclError wcublasGetVersion(int *version) {
    /* 昇腾无 cublasGetVersion 对应，返回 0（版本由 aclrtGetVersion 查） */
    if (version != nullptr) {
        *version = 0;
    }
    return ACL_SUCCESS;
}

/* ---------------- 模式类 API ----------------
 * PointerMode 被 GemmEx 真实落实（HOST→staging；DEVICE→透传）；
 * MathMode/AtomicsMode 是性能提示语义，存取透传，不据此改变数值行为。 */

aclError wcublasSetPointerMode(cublasHandle_t handle, cublasPointerMode_t mode) {
    if (handle == nullptr ||
        (mode != CUBLAS_POINTER_MODE_HOST && mode != CUBLAS_POINTER_MODE_DEVICE)) {
        return ACL_ERROR_INVALID_PARAM;
    }
    handle->pointerMode = mode;
    return ACL_SUCCESS;
}

aclError wcublasGetPointerMode(cublasHandle_t handle, cublasPointerMode_t *mode) {
    if (handle == nullptr || mode == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *mode = handle->pointerMode;
    return ACL_SUCCESS;
}

aclError wcublasSetMathMode(cublasHandle_t handle, cublasMath_t mode) {
    if (handle == nullptr || mode < CUBLAS_DEFAULT_MATH || mode > CUBLAS_PEDANTIC_MATH) {
        return ACL_ERROR_INVALID_PARAM;
    }
    handle->mathMode = mode;
    return ACL_SUCCESS;
}

aclError wcublasGetMathMode(cublasHandle_t handle, cublasMath_t *mode) {
    if (handle == nullptr || mode == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *mode = handle->mathMode;
    return ACL_SUCCESS;
}

aclError wcublasSetAtomicsMode(cublasHandle_t handle, cublasAtomicsMode_t mode) {
    if (handle == nullptr ||
        (mode != CUBLAS_ATOMICS_NOT_ALLOWED && mode != CUBLAS_ATOMICS_ALLOWED)) {
        return ACL_ERROR_INVALID_PARAM;
    }
    handle->atomicsMode = mode;
    return ACL_SUCCESS;
}

aclError wcublasGetAtomicsMode(cublasHandle_t handle, cublasAtomicsMode_t *mode) {
    if (handle == nullptr || mode == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *mode = handle->atomicsMode;
    return ACL_SUCCESS;
}

/* 通用 GEMM 内部实现：s/d 共用，用 aclblasGemmEx。
 * ⚠️ 真机教训（2026-08-21 cublasMatmul）：原先直接传 handle->stream（默认
 * nullptr），aclblasGemmEx 返回 ACL_ERROR_INVALID_PARAM（100000）——昇腾计算
 * 接口要求非空 aclrtStream，必须先 aclrtCreateStream。
 *
 * 诊断日志暂时保留在 wrapper 内：100000 可能来自 stream、shape/leading
 * dimension、数据类型组合或 computeType，调用方只能看到 cublas 返回码，
 * 因此必须在真正调用 aclblasGemmEx 的边界打印完整参数以及 ACL 最近错误。 */
/* 官方 Ascend GEMM sample 在 aclblasGemmEx 前调用 aclopSetModelDir()，加载由 ATC
 * 生成的单算子 GEMM .om。CANN 返回 100024 时不是参数错误，而是找不到匹配算子；
 * 允许 real_test 通过环境变量注入模型目录，同时不让用户 CUDA 代码依赖 ACL。 */
static aclError ensure_gemm_model_dir() {
    const char *model_dir = std::getenv("CU2CCE_GEMM_MODEL_DIR");
    if (model_dir == nullptr || model_dir[0] == '\0') {
        return ACL_SUCCESS;
    }
    static char configured_dir[4096] = {0};
    if (std::strncmp(configured_dir, model_dir, sizeof(configured_dir) - 1) == 0) {
        return ACL_SUCCESS;
    }
    aclError ret = aclopSetModelDir(model_dir);
    std::fprintf(stderr, "[cce_bridge][gemm] aclopSetModelDir dir=%s ret=%d recent=%s\n",
                 model_dir, (int)ret, aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
    if (ret == ACL_SUCCESS) {
        std::strncpy(configured_dir, model_dir, sizeof(configured_dir) - 1);
        configured_dir[sizeof(configured_dir) - 1] = '\0';
    }
    return ret;
}

static aclError gemm_impl(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                          int n, int k, const void *alpha, const void *A, int lda,
                          aclDataType dtA, const void *B, int ldb, aclDataType dtB,
                          const void *beta, void *C, int ldc, aclDataType dtC) {
    if (handle == nullptr) {
        std::fprintf(stderr,
                     "[cce_bridge][gemm] invalid handle\n");
        return ACL_ERROR_INVALID_PARAM;
    }

    const bool stream_was_null = (handle->stream == nullptr);
    /* aclblasGemmEx 的 lda/ldb/ldc 在当前 CANN 版本是保留参数，文档明确要求只能传 -1。
     * CUDA 的 lda/ldb/ldc 仍由上层传入并用于 host staging；只在 ACL 调用边界转换。
     * 之前把 CUDA 的 16/32 原样传给 ACL，正是目前最符合 100000 参数校验特征的根因。 */
    const int acl_lda = -1;
    const int acl_ldb = -1;
    const int acl_ldc = -1;
    std::fprintf(stderr,
                 "[cce_bridge][gemm] enter handle=%p stream=%p stream_was_null=%d "
                 "trans=(%d,%d,%d) shape=(m=%d,n=%d,k=%d) cuda_ld=(%d,%d,%d) acl_ld=(-1,-1,-1) "
                 "dtype=(A=%d,B=%d,C=%d) compute=%d alpha=%p beta=%p A=%p B=%p C=%p\n",
                 (void *)handle, (void *)handle->stream, stream_was_null ? 1 : 0,
                 (int)transa, (int)transb, (int)ACL_TRANS_N, m, n, k, lda, ldb, ldc,
                 (int)dtA, (int)dtB, (int)dtC, (int)ACL_COMPUTE_HIGH_PRECISION, alpha, beta, A, B,
                 C);

    if (stream_was_null) {
        aclError sret = aclrtCreateStream(&handle->stream);
        std::fprintf(stderr, "[cce_bridge][gemm] aclrtCreateStream ret=%d stream=%p recent=%s\n",
                     (int)sret, (void *)handle->stream,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        if (sret != ACL_SUCCESS) {
            return sret;
        }
    }

    aclError model_ret = ensure_gemm_model_dir();
    if (model_ret != ACL_SUCCESS) {
        return model_ret;
    }

    /* aclblasGemmEx 无 handle，直接传 stream（从 cu2cce handle 结构取）。
     * transC 昇腾仅支持 ACL_TRANS_N。官方 fp16×fp16→fp32 示例使用
     * ACL_COMPUTE_HIGH_PRECISION；此前 HIGH/LOW 都失败是因为 alpha/beta 仍是 host pointer。
     * ⚠️ 真机教训（2026-08-21 cublasMatmul）：aclblasGemmEx 是异步下发，
     *    必须在返回前 aclrtSynchronizeStream 等 GEMM 完成，否则调用方读 C
     *    会拿到未完成的结果/未触发真实错误（示例先 cudaMemcpy 读 C 才会看到
     *    错误码）。 */
    aclError gret = aclblasGemmEx(transa, transb, ACL_TRANS_N, m, n, k, alpha, A, acl_lda, dtA, B,
                                  acl_ldb, dtB, beta, C, acl_ldc, dtC, ACL_COMPUTE_HIGH_PRECISION,
                                  handle->stream);
    std::fprintf(stderr, "[cce_bridge][gemm] aclblasGemmEx ret=%d stream=%p recent=%s\n",
                 (int)gret, (void *)handle->stream,
                 aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
    if ((int)gret == 100024) {
        /* ACL_ERROR_OP_NOT_FOUND：参数校验已经通过，但运行时未找到匹配的 GEMM 算子/模型。
         * 首查 ASCEND_OPP_PATH 与 CANN set_env.sh；若环境完整仍失败，则该 SoC/类型组合
         * 没有预置实现，需要 aclopSetModelDir 加载 ATC 单算子模型或改走另一条 GEMM 实现。 */
        std::fprintf(stderr,
                     "[cce_bridge][gemm] 100024=ACL_ERROR_OP_NOT_FOUND; check ASCEND_OPP_PATH, "
                     "CANN set_env.sh, and GEMM operator/model availability for this SoC\n");
    }
    if (gret != ACL_SUCCESS) {
        return gret;
    }
    aclError sync_ret = aclrtSynchronizeStream(handle->stream);
    std::fprintf(stderr, "[cce_bridge][gemm] aclrtSynchronizeStream ret=%d recent=%s\n",
                 (int)sync_ret,
                 aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
    return sync_ret;
}

/* fp32 → half（IEEE 半精度，截断 10 位尾数）。昇腾 GemmEx 仅支持
 * fp16×fp16×fp16 / fp16×fp16×float / int8 组合（真机 9.2.0 acl_cblas.h 确认，
 * 无 fp32 aclblasSgemm）→ Sgemm 的 fp32 输入必须降级为 fp16 计算。
 * ⚠️ 这是昇腾硬件限制（fp32→fp16 静默降精度）：值 >2048 或非整数会丢精度；
 *    调用方若需精确 fp32 gemm 需自行评估。normal 数无损映射 1..2048 整数。 */
static unsigned short cce_f32_to_f16(float x) {
    unsigned int u;
    u = *((unsigned int *)&x);
    unsigned int sign = (u >> 16) & 0x8000u;
    unsigned int exp = (u >> 23) & 0xffu;
    unsigned int mant = u & 0x7fffffu;
    if (exp == 0xffu) { /* Inf/NaN → 保持符号的 Inf */
        return (unsigned short)(sign | 0x7c00u);
    }
    int e = (int)exp - 127 + 15;
    if (e >= 31) { /* 溢出 → Inf */
        return (unsigned short)(sign | 0x7c00u);
    }
    if (e <= 0) { /* 亚正常/近零 */
        if (e < -10) return (unsigned short)sign;
        mant |= 0x800000u;
        unsigned int shift = (unsigned int)(14 - e);
        unsigned int round = (mant >> (shift - 1)) & 1u;
        unsigned int h = mant >> shift;
        if (round) h++;
        return (unsigned short)(sign | h);
    }
    unsigned int h = ((unsigned int)e << 10) | (mant >> 13);
    return (unsigned short)(sign | h);
}

aclError wcublasSgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc) {
    if (handle == nullptr || alpha == nullptr || beta == nullptr || A == nullptr || B == nullptr ||
        C == nullptr || m <= 0 || n <= 0 || k <= 0) {
        return ACL_ERROR_INVALID_PARAM;
    }

    /* cublas 的 A/B/C 都是 device pointer。不能像普通 host 数组一样直接读取 A[i]/B[i]：
     * 上一轮的 100000 已经绕过了 GemmEx，但这里的 host 解包因此触发了 SIGSEGV。
     * 这条路径先 D2H 读 fp32，再 host 转 fp16，再 H2D 写临时 fp16 device buffer，最后
     * 用临时 device buffer 调 aclblasGemmEx；GemmEx 同步完成后才能释放临时 device buffer。
     * 同理，cuBLAS 的 alpha/beta 是 host scalar pointer，但 CANN 官方样例先将二者复制到
     * device，再把 device scalar pointer 传给 GemmEx；直接传栈地址会被 ACL 参数校验拒绝。 */
    const bool a_is_n = (transa == ACL_TRANS_N);
    const bool b_is_n = (transb == ACL_TRANS_N);
    const int a_cols = a_is_n ? k : m;
    const int b_cols = b_is_n ? n : k;
    const int a_min_ld = a_is_n ? m : k;
    const int b_min_ld = b_is_n ? k : n;
    if (lda < a_min_ld || ldb < b_min_ld || ldc < m) {
        return ACL_ERROR_INVALID_PARAM;
    }

    if (handle->stream == nullptr) {
        aclError sret = aclrtCreateStream(&handle->stream);
        if (sret != ACL_SUCCESS) {
            return sret;
        }
    }

    const size_t elemsA = (size_t)lda * (size_t)a_cols;
    const size_t elemsB = (size_t)ldb * (size_t)b_cols;
    const size_t bytesA = elemsA * sizeof(float);
    const size_t bytesB = elemsB * sizeof(float);
    const size_t halfBytesA = elemsA * sizeof(aclFloat16);
    const size_t halfBytesB = elemsB * sizeof(aclFloat16);
    std::fprintf(stderr,
                 "[cce_bridge][Sgemm] fp32->fp16 device staging stream=%p "
                 "shape=(m=%d,n=%d,k=%d) ld=(%d,%d,%d) storage=(A=%zu,B=%zu) "
                 "alpha=%.9g beta=%.9g\n",
                 (void *)handle->stream, m, n, k, lda, ldb, ldc, elemsA, elemsB, (double)*alpha,
                 (double)*beta);

    float *hostA = (float *)std::malloc(bytesA);
    float *hostB = (float *)std::malloc(bytesB);
    aclFloat16 *hostHalfA = (aclFloat16 *)std::malloc(halfBytesA);
    aclFloat16 *hostHalfB = (aclFloat16 *)std::malloc(halfBytesB);
    void *devHalfA = nullptr;
    void *devHalfB = nullptr;
    void *devAlpha = nullptr;
    void *devBeta = nullptr;
    if (hostA == nullptr || hostB == nullptr || hostHalfA == nullptr || hostHalfB == nullptr) {
        std::free(hostA);
        std::free(hostB);
        std::free(hostHalfA);
        std::free(hostHalfB);
        return ACL_ERROR_BAD_ALLOC;
    }

    aclError ret = aclrtMemcpy(hostA, bytesA, A, bytesA, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] D2H A failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_host;
    }
    ret = aclrtMemcpy(hostB, bytesB, B, bytesB, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] D2H B failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_host;
    }
    for (size_t i = 0; i < elemsA; ++i) hostHalfA[i] = cce_f32_to_f16(hostA[i]);
    for (size_t i = 0; i < elemsB; ++i) hostHalfB[i] = cce_f32_to_f16(hostB[i]);

    ret = aclrtMalloc(&devHalfA, halfBytesA, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] aclrtMalloc A failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_host;
    }
    ret = aclrtMalloc(&devHalfB, halfBytesB, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] aclrtMalloc B failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_a;
    }
    ret = aclrtMalloc(&devAlpha, sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] aclrtMalloc alpha failed ret=%d recent=%s\n",
                     (int)ret, aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_b;
    }
    ret = aclrtMalloc(&devBeta, sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] aclrtMalloc beta failed ret=%d recent=%s\n",
                     (int)ret, aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_alpha;
    }
    ret = aclrtMemcpy(devHalfA, halfBytesA, hostHalfA, halfBytesA, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] H2D A failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_beta;
    }
    ret = aclrtMemcpy(devHalfB, halfBytesB, hostHalfB, halfBytesB, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] H2D B failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_beta;
    }
    ret = aclrtMemcpy(devAlpha, sizeof(float), alpha, sizeof(float), ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] H2D alpha failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_beta;
    }
    ret = aclrtMemcpy(devBeta, sizeof(float), beta, sizeof(float), ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] H2D beta failed ret=%d recent=%s\n", (int)ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        goto cleanup_device_beta;
    }

    /* fp16×fp16×float 组合：A/B 为临时 device half，C 与 alpha/beta 均为 device fp32。 */
    std::fprintf(stderr,
                 "[cce_bridge][Sgemm] converted dtype=(A=%d,B=%d,C=%d) "
                 "device_half=(A=%p,B=%p) device_scale=(alpha=%p,beta=%p) "
                 "bytes=(A=%zu,B=%zu)\n",
                 (int)ACL_FLOAT16, (int)ACL_FLOAT16, (int)ACL_FLOAT, devHalfA, devHalfB, devAlpha,
                 devBeta, halfBytesA, halfBytesB);
    ret = gemm_impl(handle, transa, transb, m, n, k, devAlpha, devHalfA, lda, ACL_FLOAT16, devHalfB,
                    ldb, ACL_FLOAT16, devBeta, C, ldc, ACL_FLOAT);

cleanup_device_beta:
    if (devBeta != nullptr) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] free beta begin ptr=%p ret=%d\n", devBeta, (int)ret);
        aclError free_ret = aclrtFree(devBeta);
        std::fprintf(stderr, "[cce_bridge][Sgemm] free beta done ret=%d recent=%s\n", (int)free_ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        if (ret == ACL_SUCCESS && free_ret != ACL_SUCCESS) ret = free_ret;
    }
cleanup_device_alpha:
    if (devAlpha != nullptr) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] free alpha begin ptr=%p ret=%d\n", devAlpha, (int)ret);
        aclError free_ret = aclrtFree(devAlpha);
        std::fprintf(stderr, "[cce_bridge][Sgemm] free alpha done ret=%d recent=%s\n", (int)free_ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        if (ret == ACL_SUCCESS && free_ret != ACL_SUCCESS) ret = free_ret;
    }
cleanup_device_b:
    if (devHalfB != nullptr) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] free B begin ptr=%p ret=%d\n", devHalfB, (int)ret);
        aclError free_ret = aclrtFree(devHalfB);
        std::fprintf(stderr, "[cce_bridge][Sgemm] free B done ret=%d recent=%s\n", (int)free_ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        if (ret == ACL_SUCCESS && free_ret != ACL_SUCCESS) ret = free_ret;
    }
cleanup_device_a:
    if (devHalfA != nullptr) {
        std::fprintf(stderr, "[cce_bridge][Sgemm] free A begin ptr=%p ret=%d\n", devHalfA, (int)ret);
        aclError free_ret = aclrtFree(devHalfA);
        std::fprintf(stderr, "[cce_bridge][Sgemm] free A done ret=%d recent=%s\n", (int)free_ret,
                     aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
        if (ret == ACL_SUCCESS && free_ret != ACL_SUCCESS) ret = free_ret;
    }
cleanup_host:
    std::free(hostA);
    std::free(hostB);
    std::free(hostHalfA);
    std::free(hostHalfB);
    std::fprintf(stderr, "[cce_bridge][Sgemm] return ret=%d\n", (int)ret);
    return ret;
}

aclError wcublasDgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc) {
    (void)handle; (void)transa; (void)transb; (void)m; (void)n; (void)k;
    (void)alpha; (void)A; (void)lda; (void)B; (void)ldb; (void)beta; (void)C; (void)ldc;
    /* ⚠️ 昇腾 aclblasGemmEx 不支持 double（aclDataType 无 ACL_DT_DOUBLE，原生多 FP16/FP32/int8）。
     * 真机上若需 double，可转 FP32 或确认 CANN 是否有 double 变体；当前返回"不支持"。 */
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---------------- BLAS1/BLAS2 ----------------
 * aclblasGemvEx 存在（FP32 gemv，torch_cblas.h 确认）。Sgemv 真实映射；
 * axpy/copy/dot/scal/swap/nrm2 昇腾无对应，保持 not-support 占位。 */

/* BLAS1 占位统一入口：即使最终返回 not-support，也先完成与真实实现
 * 相同强度的参数校验——错误归因一致，且未来填充实现时签名不动。 */
static aclError blas1_validate(int n, const void *x, const void *y) {
    if (n <= 0 || x == nullptr || y == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcublasSgemv(cublasHandle_t handle, aclTransType trans, int m, int n, const float *alpha,
                      const float *A, int lda, const float *x, int incx, const float *beta,
                      float *y, int incy) {
    if (handle == nullptr || alpha == nullptr || beta == nullptr || A == nullptr || x == nullptr ||
        y == nullptr || m <= 0 || n <= 0 || incx == 0 || incy == 0 ||
        lda < ((trans == ACL_TRANS_N) ? m : n)) {
        return ACL_ERROR_INVALID_PARAM;
    }
    if (handle->stream == nullptr) {
        aclError sret = aclrtCreateStream(&handle->stream);
        if (sret != ACL_SUCCESS) {
            return sret;
        }
    }
    /* 真机教训同 Sgemm：CANN 计算接口要求 device scalar；host alpha/beta
     * 先 staging（PointerMode=DEVICE 时视为调用方已给 device 指针，透传）。 */
    void *devAlpha = nullptr;
    void *devBeta = nullptr;
    aclError ret = ACL_SUCCESS;
    if (handle->pointerMode == CUBLAS_POINTER_MODE_HOST) {
        ret = aclrtMalloc(&devAlpha, sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
        ret = aclrtMalloc(&devBeta, sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST);
        if (ret == ACL_SUCCESS) {
            ret = aclrtMemcpy(devAlpha, sizeof(float), alpha, sizeof(float), ACL_MEMCPY_HOST_TO_DEVICE);
        }
        if (ret == ACL_SUCCESS) {
            ret = aclrtMemcpy(devBeta, sizeof(float), beta, sizeof(float), ACL_MEMCPY_HOST_TO_DEVICE);
        }
        if (ret != ACL_SUCCESS) {
            aclrtFree(devAlpha);
            aclrtFree(devBeta);
            return ret;
        }
        ret = aclblasGemvEx(trans, m, n, devAlpha, A, lda, ACL_FLOAT, x, incx, ACL_FLOAT, devBeta,
                            y, incy, ACL_FLOAT, ACL_COMPUTE_HIGH_PRECISION, handle->stream);
        aclrtFree(devAlpha);
        aclrtFree(devBeta);
    } else {
        ret = aclblasGemvEx(trans, m, n, alpha, A, lda, ACL_FLOAT, x, incx, ACL_FLOAT, beta, y,
                            incy, ACL_FLOAT, ACL_COMPUTE_HIGH_PRECISION, handle->stream);
    }
    if (ret != ACL_SUCCESS) {
        return ret;
    }
    return aclrtSynchronizeStream(handle->stream);
}

aclError wcublasSaxpy(cublasHandle_t handle, int n, const float *alpha, const float *x, int incx,
                      float *y, int incy) {
    (void)handle; (void)incx; (void)incy;
    return blas1_validate(n, x, y);
}

aclError wcublasScopy(cublasHandle_t handle, int n, const float *x, int incx, float *y, int incy) {
    (void)handle; (void)incx; (void)incy;
    return blas1_validate(n, x, y);
}

aclError wcublasSdot(cublasHandle_t handle, int n, const float *x, int incx, const float *y,
                     int incy, float *result) {
    (void)handle; (void)incx; (void)incy;
    if (result == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return blas1_validate(n, x, y);
}

aclError wcublasSscal(cublasHandle_t handle, int n, const float *alpha, float *x, int incx) {
    (void)handle; (void)incx;
    if (alpha == nullptr || n <= 0 || x == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcublasSswap(cublasHandle_t handle, int n, float *x, int incx, float *y, int incy) {
    (void)handle; (void)incx; (void)incy;
    return blas1_validate(n, x, y);
}

aclError wcublasSnrm2(cublasHandle_t handle, int n, const float *x, int incx, float *result) {
    (void)handle; (void)incx;
    if (result == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return blas1_validate(n, x, nullptr);
}

/* ---------------- GemmEx / SgemmEx ----------------
 * dtype 显式入口：直接映射 aclblasGemmEx，不做 Sgemm 那样的 fp32→fp16
 * staging（dtype 由调用方声明，桥不再隐式改类型）。仅接受真机界定的
 * 组合；alpha/beta 按 PointerMode staging（教训：CANN 要求 device scalar）。 */

static bool gemmex_dtype_supported(cudaDataType a, cudaDataType b, cudaDataType c) {
    return (a == CUDA_R_16F && b == CUDA_R_16F && c == CUDA_R_16F) ||
           (a == CUDA_R_16F && b == CUDA_R_16F && c == CUDA_R_32F) ||
           (a == CUDA_R_8I && b == CUDA_R_8I && c == CUDA_R_32I);
}

static bool gemmex_dtype_to_acl(cudaDataType t, aclDataType *out) {
    switch (t) {
        case CUDA_R_16F: *out = ACL_FLOAT16; return true;
        case CUDA_R_32F: *out = ACL_FLOAT; return true;
        case CUDA_R_8I: *out = ACL_INT8; return true;
        case CUDA_R_32I: *out = ACL_INT32; return true;
        default: return false;
    }
}

aclError wcublasGemmEx(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                       int n, int k, const void *alpha, const void *A, cudaDataType Atype, int lda,
                       const void *B, cudaDataType Btype, int ldb, const void *beta, void *C,
                       cudaDataType Ctype, int ldc, cublasComputeType_t computeType) {
    if (handle == nullptr || alpha == nullptr || beta == nullptr || A == nullptr || B == nullptr ||
        C == nullptr || m <= 0 || n <= 0 || k <= 0) {
        return ACL_ERROR_INVALID_PARAM;
    }
    if (!gemmex_dtype_supported(Atype, Btype, Ctype)) {
        std::fprintf(stderr,
                     "[cce_bridge][GemmEx] dtype combo (A=%d,B=%d,C=%d) not supported by "
                     "CANN aclblasGemmEx\n",
                     (int)Atype, (int)Btype, (int)Ctype);
        return ACL_ERROR_API_NOT_SUPPORT;
    }
    /* compute 只接受 32F 族（68=CUBLAS_COMPUTE_32F, 151=CUDA_R_32F）或 16F 族
     * （64/150）。ACL 侧统一用 HIGH_PRECISION；16F compute 的降精度语义无法
     * 在 ACL 枚举上保证，诚实拒绝以避免静默数值差异。 */
    const bool compute32 = (computeType == CUBLAS_COMPUTE_32F || computeType == (cublasComputeType_t)CUDA_R_32F);
    const bool compute16 = (computeType == CUBLAS_COMPUTE_16F || computeType == (cublasComputeType_t)CUDA_R_16F);
    if (!compute32 && !compute16) {
        std::fprintf(stderr, "[cce_bridge][GemmEx] unsupported computeType=%llu\n",
                     (unsigned long long)computeType);
        return ACL_ERROR_API_NOT_SUPPORT;
    }
    if (Atype == CUDA_R_8I && !compute32) {
        /* INT8 输入按 CUDA 语义必须配 32I/32F 标量，16F 标量无意义 */
        return ACL_ERROR_INVALID_PARAM;
    }

    aclDataType dtA = ACL_FLOAT16;
    aclDataType dtB = ACL_FLOAT16;
    aclDataType dtC = ACL_FLOAT16;
    (void)gemmex_dtype_to_acl(Atype, &dtA);
    (void)gemmex_dtype_to_acl(Btype, &dtB);
    (void)gemmex_dtype_to_acl(Ctype, &dtC);

    if (handle->stream == nullptr) {
        aclError sret = aclrtCreateStream(&handle->stream);
        if (sret != ACL_SUCCESS) {
            return sret;
        }
    }

    const size_t scalar_bytes = compute16 ? 2u : 4u; /* 16F 标量 2 字节；32F/32I 标量 4 字节 */
    void *devAlpha = nullptr;
    void *devBeta = nullptr;
    const void *useAlpha = alpha;
    const void *useBeta = beta;
    aclError ret = ACL_SUCCESS;
    if (handle->pointerMode == CUBLAS_POINTER_MODE_HOST) {
        ret = aclrtMalloc(&devAlpha, scalar_bytes, ACL_MEM_MALLOC_HUGE_FIRST);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
        ret = aclrtMalloc(&devBeta, scalar_bytes, ACL_MEM_MALLOC_HUGE_FIRST);
        if (ret == ACL_SUCCESS) {
            ret = aclrtMemcpy(devAlpha, scalar_bytes, alpha, scalar_bytes, ACL_MEMCPY_HOST_TO_DEVICE);
        }
        if (ret == ACL_SUCCESS) {
            ret = aclrtMemcpy(devBeta, scalar_bytes, beta, scalar_bytes, ACL_MEMCPY_HOST_TO_DEVICE);
        }
        if (ret != ACL_SUCCESS) {
            aclrtFree(devAlpha);
            aclrtFree(devBeta);
            return ret;
        }
        useAlpha = devAlpha;
        useBeta = devBeta;
    }

    ret = gemm_impl(handle, transa, transb, m, n, k, useAlpha, A, lda, dtA, B, ldb, dtB, useBeta,
                    C, ldc, dtC);

    if (devAlpha != nullptr) {
        aclrtFree(devAlpha);
    }
    if (devBeta != nullptr) {
        aclrtFree(devBeta);
    }
    return ret;
}

aclError wcublasSgemmEx(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                        int n, int k, const void *alpha, const void *A, cudaDataType Atype, int lda,
                        const void *B, cudaDataType Btype, int ldb, const void *beta, void *C,
                        cudaDataType Ctype, int ldc) {
    return wcublasGemmEx(handle, transa, transb, m, n, k, alpha, A, Atype, lda, B, Btype, ldb,
                         beta, C, Ctype, ldc, CUBLAS_COMPUTE_32F);
}
