#!/bin/bash
# mock nvcc 入口：把 nvcc 调用转给 mock_nvcc.py（回答 CMake 探测，其余透传 cucc）
exec python3 "$CUCC_PATH/tools/mock/mock_nvcc.py" "$@"
