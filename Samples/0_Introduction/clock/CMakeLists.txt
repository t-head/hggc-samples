# ---------------------------------------------------------------------------
# Target 1: clock  (Runtime API, kernel + main in a single .hg)
# ---------------------------------------------------------------------------
add_executable(clock clock.hg)
target_include_directories(clock PRIVATE ${COMMON_DIR})

# ---------------------------------------------------------------------------
# Target 2: clock_runtime_compile  (HGRTC JIT)
# ---------------------------------------------------------------------------
add_executable(clock_runtime_compile clock_runtime_compile.cpp clock_kernel.hg)
target_include_directories(clock_runtime_compile PRIVATE ${COMMON_DIR})
target_link_libraries(clock_runtime_compile PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source to build directory (runtime JIT requires source)
add_custom_command(TARGET clock_runtime_compile POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/clock_kernel.hg
        $<TARGET_FILE_DIR:clock_runtime_compile>/clock_kernel.hg)
