add_executable(mem_strategy_bench mem_strategy_bench.hg)
target_include_directories(mem_strategy_bench PRIVATE ${COMMON_DIR})
target_link_libraries(mem_strategy_bench PRIVATE HGGC::toolkit)
