# Memory pool lifecycle: sync vs async vs pool-reuse comparison
add_executable(mem_pool_lifecycle mem_pool_lifecycle.hg)
target_include_directories(mem_pool_lifecycle PRIVATE ${COMMON_DIR})
