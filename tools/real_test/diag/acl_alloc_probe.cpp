/**
 * acl_alloc_probe —— 107002 排查：裸 ACL 分配探针（历史 runtime context 排障）
 *
 * 不经过 cucc/wrapper，直接调 aclrt*：隔离"桥（bridge_init/wrapper）vs ACL 本身"。
 * minimal 挂点（WCUDA_DEBUG=1 实测）：aclInit / aclrtGetDeviceCount / aclrtSetDevice(0)
 * 全部返回 0，紧接的 aclrtMalloc 仍报 107002（ACL_ERROR_RT_MEMORY_ALLOCATION）。
 *
 * 编译（真机执行）：
 *   g++ -std=c++17 tools/real_test/acl_alloc_probe.cpp -o /tmp/acl_probe \
 *       -I/usr/local/Ascend/latest/include \
 *       -L/usr/local/Ascend/latest/lib64 -lascendcl \
 *       -Wl,-rpath,/usr/local/Ascend/latest/lib64
 *   # 若链接报 undefined reference to 'aclInit' 等，把 -lascendcl 换成 -lacl_rt
 *
 * 运行：
 *   /tmp/acl_probe
 *
 * 判读（哪个组合 OK 就是哪条路径可用，其余排除）：
 *   A OK           → 裸 ACL 在 minimal 相同顺序下能分配，问题在 wrapper 行为（查 wcudaMalloc 路径）
 *   A FAIL / B OK  → 大页 policy 问题（§5.3，桥里改用 NORMAL_ONLY）
 *   A FAIL / C OK  → SetDevice 后需等待设备就绪（时序，桥里 SetDevice 后加握手等待）
 *   A FAIL / D OK  → 需显式 aclrtCreateContext（§4 候选根因 3，桥里 SetDevice 后建 context）
 *   E FAIL / HBM free 极小 → 环境问题：HBM 被占满（查 npu-smi / dmesg，与其他进程争显存）
 */
#include "acl/acl_rt.h"

#include <cstdio>
#include <unistd.h>

static void mem_try(int idx, const char *name, aclrtMemMallocPolicy pol, size_t size) {
    void *p = nullptr;
    aclError r = aclrtMalloc(&p, size, pol);
    printf("  malloc[%d] %-22s size=%6zu ret=%d ptr=%p\n", idx, name, size, (int)r, p);
    if (r == ACL_SUCCESS) {
        printf("  malloc[%d] aclrtFree ret=%d\n", idx, (int)aclrtFree(p));
    } else {
        const char *m = aclGetRecentErrMsg();
        printf("  malloc[%d] errMsg=%s\n", idx, m != nullptr ? m : "(null)");
    }
}

int main() {
    int32_t r = aclInit(nullptr);
    printf("[init] aclInit               ret=%d\n", (int)r);

    uint32_t cnt = 0;
    r = aclrtGetDeviceCount(&cnt);
    printf("[init] aclrtGetDeviceCount   ret=%d cnt=%u\n", (int)r, cnt);

    r = aclrtSetDevice(0);
    printf("[init] aclrtSetDevice(0)     ret=%d\n", (int)r);

    /* 环境：HBM 剩余/总量（free 太小 → 显存被其他进程占满） */
    size_t freeMem = 0, totalMem = 0;
    aclError mr = aclrtGetMemInfo(ACL_HBM_MEM, &freeMem, &totalMem);
    printf("[env]  HBM ret=%d free=%zu total=%zu\n", (int)mr, freeMem, totalMem);

    /* A: 复现 minimal 路径——SetDevice 后立即 HUGE 分配 */
    mem_try(0, "A HUGE immediate", ACL_MEM_MALLOC_HUGE_FIRST, 1024);

    /* B: §5.3 大页不足假说——换普通页 */
    mem_try(1, "B NORMAL immediate", ACL_MEM_MALLOC_NORMAL_ONLY, 1024);

    /* C: 时序假说——等 500ms 再 HUGE */
    printf("sleep 500ms...\n");
    usleep(500 * 1000);
    mem_try(2, "C HUGE +500ms", ACL_MEM_MALLOC_HUGE_FIRST, 1024);

    /* D: §4 候选根因 3——显式 CreateContext 后 HUGE */
    aclrtContext ctx = nullptr;
    aclError cr = aclrtCreateContext(&ctx, 0);
    printf("[ctx]  aclrtCreateContext(0) ret=%d ctx=%p\n", (int)cr, (void *)ctx);
    mem_try(3, "D HUGE after ctx", ACL_MEM_MALLOC_HUGE_FIRST, 1024);
    mem_try(4, "E 1MB NORMAL after ctx", ACL_MEM_MALLOC_NORMAL_ONLY, 1024 * 1024);
    if (ctx != nullptr) {
        printf("[ctx]  destroy ret=%d\n", (int)aclrtDestroyContext(ctx));
    }

    printf("[done] aclrtResetDevice ret=%d\n", (int)aclrtResetDevice(0));
    printf("[done] aclFinalize    ret=%d\n", (int)aclFinalize());
    printf("probe done\n");
    return 0;
}