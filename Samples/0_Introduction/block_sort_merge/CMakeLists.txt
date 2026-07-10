# Mixed compilation: .hg kernel + .cpp host
add_executable(block_sort_merge
    parallel_sort.hg
    parallel_sort.cpp)
set_source_files_properties(parallel_sort.hg PROPERTIES LANGUAGE HG)
target_include_directories(block_sort_merge PRIVATE ${COMMON_DIR})
target_link_libraries(block_sort_merge PRIVATE HGGC::toolkit)
