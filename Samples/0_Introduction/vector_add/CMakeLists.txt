# ---------------------------------------------------------------------------
# Target 1: vector_add  (Runtime API, kernel + main in a single .hg)
# ---------------------------------------------------------------------------
add_executable(vector_add vector_add.hg)
target_include_directories(vector_add PRIVATE ${COMMON_DIR})

# ---------------------------------------------------------------------------
# Target 2: vector_add_runtime_compile  (HGRTC JIT)
# ---------------------------------------------------------------------------
add_executable(vector_add_runtime_compile runtime_compile.cpp runtime_compile_kernel.hg)
target_include_directories(vector_add_runtime_compile PRIVATE ${COMMON_DIR})
target_link_libraries(vector_add_runtime_compile PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source to build directory (required for runtime JIT)
add_custom_command(TARGET vector_add_runtime_compile POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime_compile_kernel.hg
        $<TARGET_FILE_DIR:vector_add_runtime_compile>/runtime_compile_kernel.hg)

# ---------------------------------------------------------------------------
# Target 3: vector_add_driver_api  (Driver API + offline fatbin)
# ---------------------------------------------------------------------------
hgcc_add_fatbin(vecadd_driver_api_kernel64 driver_api_kernel.hg
    INCLUDE_DIRECTORIES ${COMMON_DIR})

add_executable(vector_add_driver_api driver_api.cpp)
target_include_directories(vector_add_driver_api PRIVATE ${COMMON_DIR})
target_link_libraries(vector_add_driver_api PRIVATE HGGC::toolkit)
add_dependencies(vector_add_driver_api vecadd_driver_api_kernel64)

# fatbin must be locatable at runtime
add_custom_command(TARGET vector_add_driver_api POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${vecadd_driver_api_kernel64_OUTPUT}
        $<TARGET_FILE_DIR:vector_add_driver_api>/driver_api_kernel64.fatbin)

# ---------------------------------------------------------------------------
# Target 4: vector_add_mmap_multidevice  (VMM + multi-PPU + fatbin)
# ---------------------------------------------------------------------------
hgcc_add_fatbin(vecadd_mmap_multidevice_kernel64 mmap_multidevice_kernel.hg
    INCLUDE_DIRECTORIES ${COMMON_DIR})

add_executable(vector_add_mmap_multidevice
    mmap_multidevice.cpp
    multi_device_mmap.cpp)
target_include_directories(vector_add_mmap_multidevice PRIVATE ${COMMON_DIR})
target_link_libraries(vector_add_mmap_multidevice PRIVATE HGGC::toolkit)
add_dependencies(vector_add_mmap_multidevice vecadd_mmap_multidevice_kernel64)

# fatbin must be locatable at runtime
add_custom_command(TARGET vector_add_mmap_multidevice POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${vecadd_mmap_multidevice_kernel64_OUTPUT}
        $<TARGET_FILE_DIR:vector_add_mmap_multidevice>/mmap_multidevice_kernel64.fatbin)
