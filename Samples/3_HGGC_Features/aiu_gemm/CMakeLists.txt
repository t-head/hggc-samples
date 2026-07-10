# AIU GEMM: BF16 GEMM via AIU .swzl Bulk Tensor Copy
add_executable(aiu_gemm aiu_gemm.hg)
set_source_files_properties(aiu_gemm.hg PROPERTIES LANGUAGE HG)
target_include_directories(aiu_gemm PRIVATE ${COMMON_DIR})
target_link_libraries(aiu_gemm PRIVATE HGGC::toolkit)
