# Radix sort using Thrust library (pure .hg)
add_executable(radix_sort_thrust radixSortThrust.hg)
target_include_directories(radix_sort_thrust PRIVATE ${COMMON_DIR})
