# ---------------------------------------------------------------------------
# Target 1: atomic_intrinsics_static  (Runtime API, .hg kernel + .cpp host)
# ---------------------------------------------------------------------------
add_executable(atomic_intrinsics_static static.hg compute_gold.cpp)
target_include_directories(atomic_intrinsics_static PRIVATE
    ${COMMON_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(atomic_intrinsics_static PRIVATE HGGC::toolkit)

# ---------------------------------------------------------------------------
# Target 2: atomic_intrinsics_hgrtc  (HGRTC JIT)
# ---------------------------------------------------------------------------
add_executable(atomic_intrinsics_hgrtc runtime_compile.cpp compute_gold.cpp)
target_include_directories(atomic_intrinsics_hgrtc PRIVATE
    ${COMMON_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(atomic_intrinsics_hgrtc PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source and headers to build directory (runtime JIT requires source)
add_custom_command(TARGET atomic_intrinsics_hgrtc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/atomic_kernel.hgh
        $<TARGET_FILE_DIR:atomic_intrinsics_hgrtc>/atomic_kernel.hgh
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/atomic_kernel_rtc.hg
        $<TARGET_FILE_DIR:atomic_intrinsics_hgrtc>/atomic_kernel_rtc.hg)
