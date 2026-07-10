add_executable(unified_memory_models unified_memory_models.hg)
target_include_directories(unified_memory_models PRIVATE ${COMMON_DIR})
target_link_libraries(unified_memory_models PRIVATE HGGC::acblas gomp)
