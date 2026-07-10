# ---------------------------------------------------------------------------
# Target 1: inline_tix_static  (static compilation)
# ---------------------------------------------------------------------------
add_executable(inline_tix_static inline_tix.hg)
target_include_directories(inline_tix_static PRIVATE ${COMMON_DIR})

# ---------------------------------------------------------------------------
# Target 2: inline_tix_hgrtc  (HGRTC JIT compilation)
# ---------------------------------------------------------------------------
add_executable(inline_tix_hgrtc inline_tix_hgrtc.cpp inline_tix_kernel.hg)
target_include_directories(inline_tix_hgrtc PRIVATE ${COMMON_DIR})
target_link_libraries(inline_tix_hgrtc PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source to build directory (required for runtime JIT)
add_custom_command(TARGET inline_tix_hgrtc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/inline_tix_kernel.hg
        $<TARGET_FILE_DIR:inline_tix_hgrtc>/inline_tix_kernel.hg)
