#!/bin/bash
# ============================================================
# run_iterate_malloc.sh —— 同一进程内反复 malloc/free，观察次数对 107002 的影响
#
# 背景（第 9 轮）：wrapper 进程里第一次 aclrtMalloc 稳定 107002，裸进程
# 第一次就成功。若"第 N 次才失败/从某次起成功"，说明是进程内状态（如
# 某些一次性的全局 flag）在作怪；若每次 malloc 都失败，则说明"任何一次
# malloc 在 wrapper 进程里都失败"（不依赖次数）。
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
INC="$CANN_LATEST/include"
LIB="$CANN_LATEST/lib64"

cat > /tmp/iter_malloc.c <<'EOF'
#include "acl/acl_rt.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    printf("iter: start\n");
    int32_t r = aclInit(NULL);
    printf("iter: aclInit ret=%d\n", (int)r);
    uint32_t cnt = 0;
    r = aclrtGetDeviceCount(&cnt);
    printf("iter: cnt=%u ret=%d\n", cnt, (int)r);
    r = aclrtSetDevice(0);
    printf("iter: setdev ret=%d\n", (int)r);
    for (int i = 0; i < 6; ++i) {
        void *p = NULL;
        r = aclrtMalloc(&p, 1024, ACL_MEM_MALLOC_HUGE_FIRST);
        printf("iter: malloc[%d] ret=%d ptr=%p\n", i, (int)r, (void *)p);
        if (r == ACL_SUCCESS) {
            printf("        free ret=%d\n", (int)aclrtFree(p));
        }
    }
    printf("iter: done\n");
    return 0;
}
EOF

echo "== 用 wrapper 链接的迭代探针 =="
gcc -o /tmp/iter_malloc /tmp/iter_malloc.c \
    -I"$INC" -L"$LIB" -lascendcl \
    -Wl,-rpath,"$LIB" \
    -L"$ROOT/build/real_test/install/lib" \
    -lcce_bridge_runtime \
    -Wl,-rpath,"$ROOT/build/real_test/install/lib"
echo "编译退出码: $?"
/tmp/iter_malloc
echo "iter 运行退出码: $?"

echo ""
echo "== 对照：不链接 wrapper 的迭代探针 =="
gcc -o /tmp/iter_malloc_raw /tmp/iter_malloc.c \
    -I"$INC" -L"$LIB" -lascendcl \
    -Wl,-rpath,"$LIB"
echo "编译退出码: $?"
/tmp/iter_malloc_raw
echo "iter_raw 运行退出码: $?"