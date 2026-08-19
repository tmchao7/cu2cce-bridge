/** trunc_raw.c —— 107002 终极对照：连 libcce_bridge_runtime.so 都不链接，
 * 用纯 system 库 + printf 完整复刻 minimal 的主流程（init→printf→malloc），
 * 逐行打印，证明"同样顺序"在 wrapper 缺席时绝不可能 107002，把差异逼到
 * wrapper 内部。
 *
 * 编译运行见 tools/real_test/run_trunc_raw.sh（服务器上跑）。
 */
#include "acl/acl_rt.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("raw: before aclInit\n");
    int32_t r = aclInit(NULL);
    printf("raw: aclInit ret=%d\n", (int)r);
    uint32_t cnt = 0;
    r = aclrtGetDeviceCount(&cnt);
    printf("raw: cnt=%u ret=%d\n", cnt, (int)r);
    r = aclrtSetDevice(0);
    printf("raw: setdev ret=%d\n", (int)r);
    void *p = NULL;
    r = aclrtMalloc(&p, 1024, ACL_MEM_MALLOC_HUGE_FIRST);
    printf("raw: malloc ret=%d ptr=%p\n", (int)r, (void *)p);
    if (r == 0) {
        aclrtFree(p);
    }
    printf("raw: done\n");
    return 0;
}