# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# CMake build rules for the HG language (hgcc compiler).
#
# Compile rules use hgcc. Link rules use the host compiler (g++) with implicit
# HGGC runtime libraries — matching how CMake handles other device compiler languages.

# --- Source and object extensions ---
set(CMAKE_HG_OUTPUT_EXTENSION .o)
set(CMAKE_HG_OUTPUT_EXTENSION_REPLACE 1)

set(CMAKE_HG_SOURCE_FILE_EXTENSIONS hg)

# --- Flag prefixes ---
set(CMAKE_INCLUDE_FLAG_HG "-I")
set(CMAKE_HG_DEFINE_FLAG  "-D")
set(CMAKE_INCLUDE_SYSTEM_FLAG_HG "-isystem ")

# --- Compiler-specific configuration ---
if(CMAKE_HG_COMPILER_ID)
  include(${CMAKE_CURRENT_LIST_DIR}/Compiler/${CMAKE_HG_COMPILER_ID}-HG.cmake OPTIONAL)
endif()

# --- Build implicit link flags string ---
set(__HG_IMPLICIT_LINKS "")
foreach(_dir IN LISTS CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES)
  string(APPEND __HG_IMPLICIT_LINKS " -L\"${_dir}\"")
endforeach()
foreach(_lib IN LISTS CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES)
  if("${_lib}" MATCHES "/")
    string(APPEND __HG_IMPLICIT_LINKS " \"${_lib}\"")
  else()
    string(APPEND __HG_IMPLICIT_LINKS " -l${_lib}")
  endif()
endforeach()

# --- Shared library flags (inherit from C/CXX conventions) ---
if(NOT CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG})
endif()
if(NOT CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG_SEP)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP})
endif()
if(NOT CMAKE_SHARED_LIBRARY_RPATH_LINK_HG_FLAG)
  set(CMAKE_SHARED_LIBRARY_RPATH_LINK_HG_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG})
endif()
if(NOT DEFINED CMAKE_EXE_EXPORTS_HG_FLAG)
  set(CMAKE_EXE_EXPORTS_HG_FLAG ${CMAKE_EXE_EXPORTS_C_FLAG})
endif()
if(NOT DEFINED CMAKE_SHARED_LIBRARY_SONAME_HG_FLAG)
  set(CMAKE_SHARED_LIBRARY_SONAME_HG_FLAG ${CMAKE_SHARED_LIBRARY_SONAME_C_FLAG})
endif()
if(NOT CMAKE_EXECUTABLE_RUNTIME_HG_FLAG)
  set(CMAKE_EXECUTABLE_RUNTIME_HG_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG})
endif()
if(NOT CMAKE_EXECUTABLE_RUNTIME_HG_FLAG_SEP)
  set(CMAKE_EXECUTABLE_RUNTIME_HG_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_HG_FLAG_SEP})
endif()
if(NOT CMAKE_EXECUTABLE_RPATH_LINK_HG_FLAG)
  set(CMAKE_EXECUTABLE_RPATH_LINK_HG_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_HG_FLAG})
endif()
if(NOT DEFINED CMAKE_SHARED_LIBRARY_LINK_HG_WITH_RUNTIME_PATH)
  set(CMAKE_SHARED_LIBRARY_LINK_HG_WITH_RUNTIME_PATH ${CMAKE_SHARED_LIBRARY_LINK_C_WITH_RUNTIME_PATH})
endif()

# Shared module defaults to shared library
if(NOT CMAKE_MODULE_EXISTS)
  set(CMAKE_SHARED_MODULE_HG_FLAGS ${CMAKE_SHARED_LIBRARY_HG_FLAGS})
  set(CMAKE_SHARED_MODULE_CREATE_HG_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_HG_FLAGS})
endif()

# --- Build-type flags (first configure only) ---
set(CMAKE_HG_FLAGS_INIT "$ENV{HGFLAGS} ${CMAKE_HG_FLAGS_INIT}")

foreach(_config "" _DEBUG _RELEASE _RELWITHDEBINFO _MINSIZEREL)
  if(NOT DEFINED CMAKE_HG_FLAGS${_config})
    set(CMAKE_HG_FLAGS${_config} "${CMAKE_HG_FLAGS${_config}_INIT}"
      CACHE STRING "Flags used by the HG compiler during ${_config} builds.")
  endif()
endforeach()

# --- Architecture flags ---
if(DEFINED CMAKE_HG_ARCHITECTURES AND NOT CMAKE_HG_ARCHITECTURES STREQUAL "")
  set(_hg_arch_flags "")
  foreach(_arch IN LISTS CMAKE_HG_ARCHITECTURES)
    string(APPEND _hg_arch_flags " --gpu-architecture=${_arch}")
  endforeach()
  string(STRIP "${_hg_arch_flags}" _hg_arch_flags)
  string(APPEND CMAKE_HG_FLAGS " ${_hg_arch_flags}")
  unset(_hg_arch_flags)
endif()

# --- Implicit include directories (for IDE navigation) ---
if(CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES)
  set(CMAKE_HG_IMPLICIT_INCLUDE_DIRECTORIES ${CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES})
endif()

# --- Runtime library selection ---
# CMAKE_HG_RUNTIME_LIBRARY: SHARED (default) or NONE.
if(NOT DEFINED CMAKE_HG_RUNTIME_LIBRARY)
  set(CMAKE_HG_RUNTIME_LIBRARY "${CMAKE_HG_RUNTIME_LIBRARY_DEFAULT}")
endif()

if(CMAKE_HG_RUNTIME_LIBRARY STREQUAL "NONE")
  set(CMAKE_HG_IMPLICIT_LINK_LIBRARIES "")
elseif(CMAKE_HG_RUNTIME_LIBRARY STREQUAL "SHARED" OR CMAKE_HG_RUNTIME_LIBRARY STREQUAL "")
  set(CMAKE_HG_IMPLICIT_LINK_LIBRARIES "${CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES}")
else()
  message(WARNING "CMAKE_HG_RUNTIME_LIBRARY=${CMAKE_HG_RUNTIME_LIBRARY} is not recognized. "
    "Supported values: SHARED (default), NONE. Falling back to SHARED.")
  set(CMAKE_HG_IMPLICIT_LINK_LIBRARIES "${CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES}")
endif()
set(CMAKE_HG_IMPLICIT_LINK_DIRECTORIES "${CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES}")

# --- C++ standard handling ---
# CMake does not auto-apply CMAKE_<LANG>_STANDARD for external custom languages.
# We handle it manually: map CMAKE_HG_STANDARD to --std=c++XX.
if(DEFINED CMAKE_HG_STANDARD AND NOT CMAKE_HG_STANDARD STREQUAL "")
  set(_hg_std_supported 11 14 17 20)
  if(DEFINED CMAKE_HG${CMAKE_HG_STANDARD}_STANDARD_COMPILE_OPTION)
    string(APPEND CMAKE_HG_FLAGS " ${CMAKE_HG${CMAKE_HG_STANDARD}_STANDARD_COMPILE_OPTION}")
  elseif(CMAKE_HG_STANDARD_REQUIRED)
    message(FATAL_ERROR
      "CMAKE_HG_STANDARD=${CMAKE_HG_STANDARD} is not supported by hgcc and "
      "CMAKE_HG_STANDARD_REQUIRED is set. Supported values: ${_hg_std_supported}")
  else()
    message(WARNING "CMAKE_HG_STANDARD=${CMAKE_HG_STANDARD} is not supported by hgcc. "
      "Supported values: ${_hg_std_supported}")
  endif()
  unset(_hg_std_supported)
endif()

# --- Dependency file generation ---
# Note: CMAKE_DEPFILE_FLAGS_HG is set in Compiler/HGCC-HG.cmake with -MT support.
# Only set fallback defaults here if the compiler-specific file didn't load.
if(NOT DEFINED CMAKE_DEPFILE_FLAGS_HG)
  set(CMAKE_DEPFILE_FLAGS_HG "-MD -MF <DEP_FILE>")
endif()
if(NOT DEFINED CMAKE_HG_DEPFILE_FORMAT)
  set(CMAKE_HG_DEPFILE_FORMAT gcc)
endif()

# --- Compile rule: .hg -> .o ---
# Uses hgcc with -x hg. Host compiler forwarding via --ccbin.
# Compiler launcher (ccache/sccache) support via CMAKE_HG_COMPILER_LAUNCHER.
set(_hg_compiler_launcher "")
if(CMAKE_HG_COMPILER_LAUNCHER)
  set(_hg_compiler_launcher "${CMAKE_HG_COMPILER_LAUNCHER} ")
endif()

if(NOT CMAKE_HG_COMPILE_OBJECT)
  set(CMAKE_HG_COMPILE_OBJECT
    "${_hg_compiler_launcher}<CMAKE_HG_COMPILER> ${_CMAKE_HG_EXTRA_FLAGS} <DEFINES> <INCLUDES> <FLAGS> -x hg -c <SOURCE> -o <OBJECT>")
endif()
unset(_hg_compiler_launcher)

# --- Link rules ---
# When LINKER_LANGUAGE is HG (pure HG targets, or RDC mixed targets), link via
# hgcc. hgcc internally handles device linking and then invokes the host
# compiler, so this works for both RDC and non-RDC modes.
#
# For mixed HG+CXX targets where LINKER_LANGUAGE defaults to CXX, the CXX
# link rule is used with HGGC runtime libraries injected via
# CMAKE_HG_IMPLICIT_LINK_LIBRARIES (set below). This covers the non-RDC case.
#
# For mixed HG+CXX targets with RDC, users set LINKER_LANGUAGE HG (or use
# hgcc_enable_rdc) to force hgcc as the linker.

if(NOT CMAKE_HG_LINK_EXECUTABLE)
  set(CMAKE_HG_LINK_EXECUTABLE
    "<CMAKE_HG_COMPILER> ${_CMAKE_HG_EXTRA_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

if(NOT CMAKE_HG_CREATE_SHARED_LIBRARY)
  set(CMAKE_HG_CREATE_SHARED_LIBRARY
    "<CMAKE_HG_COMPILER> ${_CMAKE_HG_EXTRA_FLAGS} --shared <CMAKE_SHARED_LIBRARY_HG_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_HG_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
endif()

if(NOT CMAKE_HG_CREATE_SHARED_MODULE)
  set(CMAKE_HG_CREATE_SHARED_MODULE ${CMAKE_HG_CREATE_SHARED_LIBRARY})
endif()

# --- Static archive rules ---
if(NOT DEFINED CMAKE_HG_ARCHIVE_CREATE)
  set(CMAKE_HG_ARCHIVE_CREATE "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_HG_ARCHIVE_APPEND)
  set(CMAKE_HG_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_HG_ARCHIVE_FINISH)
  set(CMAKE_HG_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()

# --- Device link rules (for separable compilation / -dc mode) ---
if(NOT CMAKE_HG_DEVICE_LINK_LIBRARY)
  set(CMAKE_HG_DEVICE_LINK_LIBRARY
    "<CMAKE_HG_COMPILER> ${_CMAKE_HG_EXTRA_FLAGS} <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> ${CMAKE_HG_COMPILE_OPTIONS_PIC} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()
if(NOT CMAKE_HG_DEVICE_LINK_EXECUTABLE)
  set(CMAKE_HG_DEVICE_LINK_EXECUTABLE
    "<CMAKE_HG_COMPILER> ${_CMAKE_HG_EXTRA_FLAGS} <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> ${CMAKE_HG_COMPILE_OPTIONS_PIC} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

# --- Response file support ---
if(NOT DEFINED CMAKE_HG_USE_RESPONSE_FILE_FOR_INCLUDES)
  set(CMAKE_HG_USE_RESPONSE_FILE_FOR_INCLUDES 0)
endif()
if(NOT DEFINED CMAKE_HG_USE_RESPONSE_FILE_FOR_LIBRARIES)
  set(CMAKE_HG_USE_RESPONSE_FILE_FOR_LIBRARIES 0)
endif()
if(NOT DEFINED CMAKE_HG_USE_RESPONSE_FILE_FOR_OBJECTS)
  set(CMAKE_HG_USE_RESPONSE_FILE_FOR_OBJECTS 0)
endif()

unset(__HG_IMPLICIT_LINKS)

set(CMAKE_HG_INFORMATION_LOADED 1)
