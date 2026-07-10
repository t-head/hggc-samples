# Single .hg file (uses C++11 std::thread, no external multithreading lib)
add_executable(stream_callback stream_callback.hg)
target_include_directories(stream_callback PRIVATE ${COMMON_DIR})
target_link_libraries(stream_callback PRIVATE HGGC::toolkit)
