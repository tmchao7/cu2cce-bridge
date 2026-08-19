/*
 * probe_device.cpp —— 运行时设备探测（env_probe.sh 第 5 节调用）
 *
 * 用途：判断当前环境是否有可用 NPU 设备（无 npu-smi 时的手动等价物）。
 * 只读查询，不加载/运行 kernel：
 *   aclInit → aclrtGetDeviceCount → aclrtGetSocName → aclFinalize
 *
 * 编译（env_probe.sh 自动做）：
 *   g++ probe_device.cpp -o probe_device -I $ASCEND_INCLUDE -L $ASCEND_LIB -lascendcl
 *
 * 退出码：0 = 有设备；1 = 无设备（aclInit/GetDeviceCount 失败或 count=0）；2 = 用法错误
 */
#include <acl/acl_rt.h>
#include <acl/acl_base.h>
#include <cstdio>

int main() {
    aclError e = aclInit(nullptr);
    if (e != ACL_SUCCESS) {
        printf("probe_device: aclInit failed, ret=%d\n", (int)e);
        return 1;
    }

    uint32_t count = 0;
    e = aclrtGetDeviceCount(&count);
    if (e != ACL_SUCCESS || count == 0) {
        printf("probe_device: no available device (ret=%d, count=%u)\n", (int)e, count);
        aclFinalize();
        return 1;
    }
    printf("probe_device: device count = %u\n", count);

    const char *soc = aclrtGetSocName();
    printf("probe_device: soc name = %s\n", soc != nullptr ? soc : "(null)");

    for (uint32_t d = 0; d < count; ++d) {
        int32_t cur = 0;
        if (aclrtSetDevice((int32_t)d) == ACL_SUCCESS) {
            aclrtGetDevice(&cur);
            const char *s2 = aclrtGetSocName();
            printf("probe_device: device %u (current=%d) soc=%s\n", d, cur,
                   s2 != nullptr ? s2 : "(null)");
            aclrtResetDevice((int32_t)d);
        }
    }

    aclFinalize();
    printf("probe_device: OK\n");
    return 0;
}
