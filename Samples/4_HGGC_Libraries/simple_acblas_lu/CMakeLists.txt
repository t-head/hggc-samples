add_executable(simple_acblas_lu simple_acblas_lu.cpp)
set_source_files_properties(simple_acblas_lu.cpp PROPERTIES LANGUAGE HG)
target_include_directories(simple_acblas_lu PRIVATE ${COMMON_DIR})
target_link_libraries(simple_acblas_lu PRIVATE HGGC::toolkit HGGC::acblas)
