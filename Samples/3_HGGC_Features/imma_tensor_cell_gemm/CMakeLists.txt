# INT8 GEMM via TIX Tensor Cell instructions (m16n16k32, double-buffer)
add_executable(imma_tensor_cell_gemm tix_tc_gemm_int8.hg)
target_include_directories(imma_tensor_cell_gemm PRIVATE ${COMMON_DIR})
