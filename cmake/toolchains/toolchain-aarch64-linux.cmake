# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross-compilers
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_AR aarch64-linux-gnu-ar)
set(CMAKE_RANLIB aarch64-linux-gnu-ranlib)

# Indicate cross-compiling.
set(CMAKE_CROSSCOMPILING TRUE)

# Set HGGC compiler flags
set(CMAKE_HGGC_FLAGS "${CMAKE_HGGC_FLAGS} -ccbin ${CMAKE_CXX_COMPILER}" CACHE STRING "" FORCE)

# Use a local sysroot copy
if(DEFINED TARGET_FS)
    set(CMAKE_SYSROOT "${TARGET_FS}")
    list(APPEND CMAKE_FIND_ROOT_PATH
        "/usr/local/hggc/targets/aarch64-linux"
    )

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    
    set(CMAKE_HGGC_FLAGS "${CMAKE_HGGC_FLAGS} -Xcompiler --sysroot=${TARGET_FS}")

    set(LIB_PATHS
        "${TARGET_FS}/usr/lib/"
        "${TARGET_FS}/usr/lib/aarch64-linux-gnu"
        "${TARGET_FS}/usr/lib/aarch64-linux-gnu/hggc"
    )
    # Add rpath-link flags for all library paths
    foreach(lib_path ${LIB_PATHS})
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${lib_path}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath-link,${lib_path}")
    endforeach()
endif()
