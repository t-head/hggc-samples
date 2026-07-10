# Stream compaction (conditional filtering) with 3 progressive strategies
add_executable(stream_compact stream_compact.hg)
target_include_directories(stream_compact PRIVATE ${COMMON_DIR})
