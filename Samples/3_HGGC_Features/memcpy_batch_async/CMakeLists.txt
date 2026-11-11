add_executable(memcpy_batch_async memcpy_batch_async.hg)
target_include_directories(memcpy_batch_async PRIVATE ${COMMON_DIR})
target_link_libraries(memcpy_batch_async PRIVATE HGGC::toolkit)
