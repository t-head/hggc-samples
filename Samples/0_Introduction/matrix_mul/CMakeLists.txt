# ---------------------------------------------------------------------------
# Locate the SDK target include directory that contains cooperative_groups.h
# and hggc_pipeline.h; they are provided by the T-HEAD SAIL SDK and needed by
# the HGRTC target which loads the kernel source at runtime via findSampleAsset().
# ---------------------------------------------------------------------------
if(DEFINED ENV{PPU_SDK})
    find_path(PPU_SDK_TARGET_INCLUDE_DIR
        NAMES cooperative_groups.h hggc_pipeline.h
        PATHS $ENV{PPU_SDK}/targets/x86_64-linux/include
              $ENV{PPU_SDK}/include
        NO_DEFAULT_PATH)
endif()
if(NOT PPU_SDK_TARGET_INCLUDE_DIR)
    message(WARNING
        "SDK target include directory not found via PPU_SDK; "
        "matrix_mul_runtime_compile may fail to locate headers at runtime.")
endif()

# ---------------------------------------------------------------------------
# Target 1: matrix_mul  (Runtime API, kernel + main in a single .hg)
# ---------------------------------------------------------------------------
add_executable(matrix_mul matrix_mul.hg)
target_include_directories(matrix_mul PRIVATE ${COMMON_DIR})

# ---------------------------------------------------------------------------
# Target 2: matrix_mul_runtime_compile  (HGRTC JIT)
# ---------------------------------------------------------------------------
add_executable(matrix_mul_runtime_compile runtime_compile.cpp runtime_compile_kernel.hg)
target_include_directories(matrix_mul_runtime_compile PRIVATE ${COMMON_DIR})
target_link_libraries(matrix_mul_runtime_compile PRIVATE HGGC::hgrtc HGGC::toolkit)

# Copy kernel source to build directory (runtime JIT requires source)
add_custom_command(TARGET matrix_mul_runtime_compile POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime_compile_kernel.hg
        $<TARGET_FILE_DIR:matrix_mul_runtime_compile>/runtime_compile_kernel.hg)

# Copy all SDK target headers next to the executable so that the runtime
# findSampleAsset() lookup succeeds when HGRTC compiles the kernel.
if(PPU_SDK_TARGET_INCLUDE_DIR)
    file(GLOB PPU_SDK_TARGET_HEADERS "${PPU_SDK_TARGET_INCLUDE_DIR}/*.h")
    add_custom_command(TARGET matrix_mul_runtime_compile POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PPU_SDK_TARGET_HEADERS}
            $<TARGET_FILE_DIR:matrix_mul_runtime_compile>/
        COMMENT "Copying SDK target headers next to matrix_mul_runtime_compile")
    if(IS_DIRECTORY "${PPU_SDK_TARGET_INCLUDE_DIR}/cooperative_groups")
        add_custom_command(TARGET matrix_mul_runtime_compile POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PPU_SDK_TARGET_INCLUDE_DIR}/cooperative_groups
                $<TARGET_FILE_DIR:matrix_mul_runtime_compile>/cooperative_groups
            COMMENT "Mirroring cooperative_groups/ headers next to matrix_mul_runtime_compile")
    endif()
endif()

# ---------------------------------------------------------------------------
# Target 3: matrix_mul_driver_api  (Driver API + offline fatbin)
# ---------------------------------------------------------------------------
hgcc_add_fatbin(driver_api_kernel64 driver_api_kernel.hg
    INCLUDE_DIRECTORIES ${COMMON_DIR})

add_executable(matrix_mul_driver_api driver_api.cpp)
target_include_directories(matrix_mul_driver_api PRIVATE ${COMMON_DIR})
target_link_libraries(matrix_mul_driver_api PRIVATE HGGC::toolkit)

