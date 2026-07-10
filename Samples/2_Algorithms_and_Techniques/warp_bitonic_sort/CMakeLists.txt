# Warp-level bitonic sort using __shfl_xor_sync (register-only, no shared memory)
add_executable(warp_bitonic_sort warp_bitonic_sort.hg)
target_include_directories(warp_bitonic_sort PRIVATE ${COMMON_DIR})
