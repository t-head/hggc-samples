# TF32 GEMM via TIX Tensor Cell instructions (m16n16k8, double-buffer)
add_executable(tf32_tensor_cell_gemm tix_tc_gemm_tf32.hg)
target_include_directories(tf32_tensor_cell_gemm PRIVATE ${COMMON_DIR})
