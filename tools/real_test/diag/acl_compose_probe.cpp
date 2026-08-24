/**
 * acl_compose_probe —— 107002 排查（第 5 轮）：库组合探针
 *
 * 一段完全相同的分配代码（aclInit → GetDeviceCount → SetDevice(0) →
 * aclrtMalloc(1KB,HUGE)），由 run_acl_compose_probe.sh 以 4 种链接组合构建，
 * 看哪种组合让 aclrtMalloc 从 ret=0 翻转为 107002，锁定"多带哪个库"导致分配失败。
 *
 * 背景：Part0 复刻探针（单独链接）成功；minimal（同样先内置 enumer+SetDevice，
 * 但进程里还加载了 libruntime.so + libcce_bridge_runtime.so + kernel 设备代码）
 * 稳定 107002。差异必在库组合（或 CANN 多根双加载）。
 */
#include "acl/acl_rt.h"

#include <cstdio>

int main() {
    int32_t r = aclInit(nullptr);
    printf("[init] aclInit               ret=%d\n", (int)r);

    uint32_t cnt = 0;
    r = aclrtGetDeviceCount(&cnt);
    printf("[init] aclrtGetDeviceCount   ret=%d cnt=%u\n", (int)r, cnt);

    r = aclrtSetDevice(0);
    printf("[init] aclrtSetDevice(0)     ret=%d\n", (int)r);

    void *p = nullptr;
    r = aclrtMalloc(&p, 1024, ACL_MEM_MALLOC_HUGE_FIRST);
    printf("[alloc] aclrtMalloc(1KB,HUGE) ret=%d ptr=%p\n", (int)r, (void *)p);
    if (r == ACL_SUCCESS) {
        printf("[alloc] aclrtFree            ret=%d\n", (int)aclrtFree(p));
    } else {
        const char *m = aclGetRecentErrMsg();
        printf("[alloc] errMsg=%s\n", m != nullptr ? m : "(null)");
    }
    return 0;
}