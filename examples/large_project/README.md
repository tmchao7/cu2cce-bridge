# large_project —— multi-directory / cross-target dependency / install-export
# validation carrier for the cu2cce-bridge CMake compatibility surface.
#
# Build through the bridge (on the server):
#   cmake_cce -S examples/large_project -B build/large_project
#   cmake --build build/large_project
#   ./build/large_project/large_project_app
#
# Structure:
#   top/                app/main.cpp -> large_util_lib; depends on gen_data_file
#   comp/               large_kernel_lib (one .cu; kernel.cu + kernel.h)
#   util/               large_host_lib (store.cpp), generate_kernel_data,
#                       large_gen_data (INTERFACE include for generated input)
#
# Backend boundary exercised: exactly one .cu per linked library target. The
# kernel library is the single CUDA translation unit; everything else is host
# C++ so the multi-directory graph stays within ccec's duplicate
# eager-registration constraint (see docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md).
#
# Expected run output marker: "large_project OK (multi-dir, cross-target deps, install/export)"