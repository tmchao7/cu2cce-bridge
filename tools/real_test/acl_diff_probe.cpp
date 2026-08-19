/**
 * acl_diff_probe —— 107002 排查（第 4 轮）：同一 libascendcl.so 内
 * 「探针全通过 vs minimal 稳定 107002」的差异定位（INVESTIGATION_107002.md §5.7）
 *
 * 目标：把 minimal 的调用序列与探针的调用序列逐步对齐，找出让 aclrtMalloc
 * 从「成功」翻转为「107002」的那一个差异。
 *
 * 已知事实（前 3 轮）：
 *   - minimal（WCUDA_DEBUG=1）：bridge_init(a clInit/GetDeviceCount/SetDevice 全 0)
 *     → wcudaMalloc enter → aclrtMalloc(1KB, HUGE_FIRST) ret=107002（稳定复现）
 *   - 裸 ACL 探针：aclInit → GetDeviceCount → SetDevice(0) → 5 次分配全成功
 *     （HBM free 97GB；HUGE 1KB/NORMAL 1KB/500ms 后 HUGE/建 ctx 后 HUGE/1MB NORMAL）
 *   - ldd（第 3 步若已跑）确认 minimal 加载的就是这份安装目录 .so（未附输出，
 *     默认按"同为一份 libascendcl.so"处理，所以差异必在进程内调用序列）
 *
 * 结构：Part 0 逐行复刻 minimal 序列；Part 1 从复刻序列出发逐步
 * 增/删差异动作，每步单独 malloc 并打 ret。判读见底部注释。
 */
#include "acl/acl_rt.h"

#include <cstdio>
#include <cstring>

/* 打印某次分配结果 */
static void try_alloc(int idx, const char *name, size_t size) {
    void *p = nullptr;
    aclError r = aclrtMalloc(&p, size, ACL_MEM_MALLOC_HUGE_FIRST);
    printf("  [%2d] %-38s ret=%d  ptr=%p\n", idx, name, (int)r, (void *)p);
    if (r == ACL_SUCCESS) {
        printf("       aclrtFree ret=%d\n", (int)aclrtFree(p));
    }
}

int main() {
    /* ================= Part 0：逐行复刻 minimal =================
     * minimal 的实际流程（wcudaMalloc 压印版）：
     *   aclInit → aclrtGetDeviceCount → aclrtSetDevice(0) → aclrtMalloc(1024) */
    printf("== Part 0: 逐行复刻 minimal 序列 ==\n");
    int32_t r = aclInit(nullptr);
    printf("  aclInit               ret=%d\n", (int)r);
    uint32_t cnt = 0;
    r = aclrtGetDeviceCount(&cnt);
    printf("  aclrtGetDeviceCount   ret=%d cnt=%u\n", (int)r, cnt);
    r = aclrtSetDevice(0);
    printf("  aclrtSetDevice(0)     ret=%d\n", (int)r);
    try_alloc(0, "A: 复刻 minimal 立即 1KB", 1024);

    /* ================= Part 1：逐步对齐差异 =================
     * 候选差异（哪个组合翻成就锁在哪个动作）：
     *   t0 = SetDevice(0) 后立即 malloc          —— 已测（A）
     *   +HBM 查询后再 malloc                    —— 探针做过、minimal 没做
     *   +一次性分配 3 块（size 与 minimal 相同）  —— minimal 只分配 1 块
     *   +分配前先 aclrtGetDevice / GetSocName   —— minimal 没做
     *   +NORMAL policy                          —— minimal 用 HUGE_FIRST */

    printf("== Part 1: 打开 HBM 空间开关（aclrtGetMemInfo），再 malloc ==\n");
    size_t freeMem = 0, totalMem = 0;
    aclError mr = aclrtGetMemInfo(ACL_HBM_MEM, &freeMem, &totalMem);
    printf("  aclrtGetMemInfo ret=%d free=%zu total=%zu\n", (int)mr, freeMem, totalMem);
    try_alloc(1, "B: HBM 查询后 1KB", 1024);

    printf("== Part 1: 查询当前 device / soc 后再 malloc ==\n");
    int32_t curDev = -1;
    r = aclrtGetDevice(&curDev);
    printf("  aclrtGetDevice ret=%d dev=%d\n", (int)r, curDev);
    const char *soc = aclrtGetSocName();
    printf("  aclrtGetSocName -> %s\n", soc != nullptr ? soc : "(null)");
    try_alloc(2, "C: 查询 device/soc 后 1KB", 1024);

    printf("== Part 1: 连续 3 次 malloc（minimal 只 1 次，vectorAdd 3 次全成功） ==\n");
    try_alloc(3, "D1: 三连分配 第1块 1KB", 1024);
    try_alloc(4, "D2: 三连分配 第2块 1KB", 1024);
    try_alloc(5, "D3: 三连分配 第3块 1KB", 1024);

    printf("== Part 1: NORMAL policy（minimal 用 HUGE_FIRST） ==\n");
    {
        void *p = nullptr;
        aclError nr = aclrtMalloc(&p, 1024, ACL_MEM_MALLOC_NORMAL_ONLY);
        printf("  [%2d] %-38s ret=%d  ptr=%p\n", 6, "E: 1KB NORMAL_ONLY", (int)nr, (void *)p);
        if (nr == ACL_SUCCESS) {
            printf("       aclrtFree ret=%d\n", (int)aclrtFree(p));
        }
    }

    printf("== Part 1: 中途 aclFinalize 后重新 init（探针方式收尾） ==\n");
    (void)aclrtResetDevice(0);
    printf("  aclrtResetDevice ret=%d\n", (int)aclrtResetDevice(0));
    printf("probe done\n");
    return 0;
}