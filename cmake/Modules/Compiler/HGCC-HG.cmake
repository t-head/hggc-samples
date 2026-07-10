# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# Compiler-specific configuration for HGCC (hgcc).
# Loaded by CMakeHGInformation.cmake when CMAKE_HG_COMPILER_ID is "HGCC".

set(CMAKE_HG_COMPILER_HAS_DEVICE_LINK_PHASE TRUE)
set(CMAKE_HG_VERBOSE_FLAG "-v")
set(CMAKE_HG_VERBOSE_COMPILE_FLAG "-Xcompiler=-v")

# --- Compilation mode flags ---
set(_CMAKE_COMPILE_AS_HG_FLAG "-x hg")
set(_CMAKE_HG_WHOLE_FLAG "-c")
set(_CMAKE_HG_RDC_FLAG "-rdc=true")

# --- Forward unknown flags to host compiler ---
set(_CMAKE_HG_EXTRA_FLAGS "-forward-unknown-to-host-compiler")

# --- Host compiler forwarding ---
if(CMAKE_HG_HOST_COMPILER)
  string(APPEND _CMAKE_HG_EXTRA_FLAGS " -ccbin=${CMAKE_HG_HOST_COMPILER}")
endif()

# --- Warning control ---
set(CMAKE_HG_COMPILE_OPTIONS_WARNING_AS_ERROR "-Werror" "all-warnings")

# --- PIC / PIE ---
set(CMAKE_HG_COMPILE_OPTIONS_PIE -Xcompiler=-fPIE)
set(CMAKE_HG_COMPILE_OPTIONS_PIC -Xcompiler=-fPIC)
set(CMAKE_HG_COMPILE_OPTIONS_VISIBILITY -Xcompiler=-fvisibility=)
set(CMAKE_SHARED_LIBRARY_HG_FLAGS -fPIC)

# --- Build-type flags ---
string(APPEND CMAKE_HG_FLAGS_INIT " ")
string(APPEND CMAKE_HG_FLAGS_DEBUG_INIT " -g -G")
string(APPEND CMAKE_HG_FLAGS_RELEASE_INIT " -O3 -DNDEBUG")
string(APPEND CMAKE_HG_FLAGS_MINSIZEREL_INIT " -O1 -DNDEBUG")
string(APPEND CMAKE_HG_FLAGS_RELWITHDEBINFO_INIT " -O2 -g --generate-line-info -DNDEBUG")

# --- Shared library creation ---
set(CMAKE_SHARED_LIBRARY_CREATE_HG_FLAGS -shared)

# --- Linker wrapper flags ---
set(CMAKE_HG_LINKER_WRAPPER_FLAG "-Wl,")
set(CMAKE_HG_LINKER_WRAPPER_FLAG_SEP ",")

set(CMAKE_HG_DEVICE_COMPILER_WRAPPER_FLAG "-Xcompiler=")
set(CMAKE_HG_DEVICE_COMPILER_WRAPPER_FLAG_SEP ",")
set(CMAKE_HG_DEVICE_LINKER_WRAPPER_FLAG "-Xlinker=")
set(CMAKE_HG_DEVICE_LINKER_WRAPPER_FLAG_SEP ",")

# --- Dependency file generation ---
set(CMAKE_DEPFILE_FLAGS_HG "-MD -MT <DEP_TARGET> -MF <DEP_FILE>")
set(CMAKE_HG_DEPFILE_FORMAT gcc)
if((NOT DEFINED CMAKE_DEPENDS_USE_COMPILER OR CMAKE_DEPENDS_USE_COMPILER)
    AND CMAKE_GENERATOR MATCHES "Makefiles|WMake")
  set(CMAKE_HG_DEPENDS_USE_COMPILER TRUE)
endif()

# --- IPO / LTO ---
set(_CMAKE_HG_IPO_SUPPORTED_BY_CMAKE YES)
set(_CMAKE_HG_IPO_MAY_BE_SUPPORTED_BY_COMPILER YES)
set(CMAKE_HG_DEVICE_LINK_OPTIONS_IPO " -dlto")

# --- C++ standard compile options ---
# hgcc supports c++11, c++14, c++17, c++20 (SDK headers require >= c++11)
set(CMAKE_HG11_STANDARD_COMPILE_OPTION "-std=c++11")
set(CMAKE_HG11_EXTENSION_COMPILE_OPTION "-std=c++11")

set(CMAKE_HG14_STANDARD_COMPILE_OPTION "-std=c++14")
set(CMAKE_HG14_EXTENSION_COMPILE_OPTION "-std=c++14")

set(CMAKE_HG17_STANDARD_COMPILE_OPTION "-std=c++17")
set(CMAKE_HG17_EXTENSION_COMPILE_OPTION "-std=c++17")

set(CMAKE_HG20_STANDARD_COMPILE_OPTION "-std=c++20")
set(CMAKE_HG20_EXTENSION_COMPILE_OPTION "-std=c++20")

# --- Runtime library options ---
# Runtime library selection: SHARED (default), NONE.
# No static runtime available in current T-HEAD SAIL SDK.
set(CMAKE_HG_RUNTIME_LIBRARY_LINK_OPTIONS_SHARED "hggc;hggcrt1")
set(CMAKE_HG_RUNTIME_LIBRARY_LINK_OPTIONS_NONE   "")
if(NOT DEFINED CMAKE_HG_RUNTIME_LIBRARY_DEFAULT)
  set(CMAKE_HG_RUNTIME_LIBRARY_DEFAULT "SHARED")
endif()

# --- Compiler launcher (ccache / sccache) ---
if(NOT CMAKE_HG_COMPILER_LAUNCHER AND DEFINED ENV{CMAKE_HG_COMPILER_LAUNCHER})
  set(CMAKE_HG_COMPILER_LAUNCHER "$ENV{CMAKE_HG_COMPILER_LAUNCHER}"
    CACHE STRING "Compiler launcher for HG.")
endif()

# --- Response file support ---
set(CMAKE_HG_RESPONSE_FILE_FLAG "--options-file ")
