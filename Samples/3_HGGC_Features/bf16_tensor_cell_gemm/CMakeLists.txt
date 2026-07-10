# BF16 GEMM via TIX Tensor Cell instructions (double-buffer pipeline)
add_executable(bf16_tensor_cell_gemm tix_tc_gemm.hg)
target_include_directories(bf16_tensor_cell_gemm PRIVATE ${COMMON_DIR})
