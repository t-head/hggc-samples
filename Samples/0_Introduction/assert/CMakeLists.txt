# ---------------------------------------------------------------------------
# Target 1: assert  (Runtime API, kernel + main in a single .hg)
# ---------------------------------------------------------------------------
add_executable(assert assert.hg)
target_include_directories(assert PRIVATE ${COMMON_DIR})

# ---------------------------------------------------------------------------
# Target 2: assert_runtime_compile  (HGRTC JIT)
# ---------------------------------------------------------------------------
add_executable(assert_runtime_compile assert_runtime_compile.cpp assert_kernel.hg)
target_include_directories(assert_runtime_compile PRIVATE ${COMMON_DIR})
target_link_libraries(assert_runtime_compile PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source to build directory (runtime JIT requires source)
add_custom_command(TARGET assert_runtime_compile POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/assert_kernel.hg
        $<TARGET_FILE_DIR:assert_runtime_compile>/assert_kernel.hg)
