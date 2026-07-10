# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
#[=======================================================================[.rst:
FindHGGCToolkit
---------------

Find the HGGC (T-HEAD SAIL SDK) toolkit and create imported targets for its libraries.

This module can be used with or without ``enable_language(HG)``. When the HG
language is enabled, ``CMAKE_HG_TOOLKIT_ROOT`` is used as the primary search
path. Otherwise, the module searches via environment variables and standard
install locations — matching the behaviour of CMake's standard toolkit find modules.

Search order for the SDK root:
  1. ``HGGCToolkit_ROOT`` (cmake variable or environment variable)
  2. ``CMAKE_HG_TOOLKIT_ROOT`` (set by enable_language(HG))
  3. ``ENV{PPU_SDK}``
  4. ``/usr/local/PPU_SDK``

Imported Targets
^^^^^^^^^^^^^^^^

``HGGC::toolkit``
  Umbrella interface target — links all core runtime dependencies.

``HGGC::hggcrt``
  HGGC runtime library (libhggcrt1).
``HGGC::hggc``
  HGGC driver library (libhggc).
``HGGC::hgrtc``
  Runtime compilation library.
``HGGC::hgrtc_builtins``
  Runtime compilation builtins.
``HGGC::hgvm``
  VM API library.
``HGGC::hgJitLink``
  JIT linker library.
``HGGC::hgJitLink_static``
  JIT linker static library.
``HGGC::acblas``
  BLAS library.
``HGGC::acblasLt``
  BLAS light-weight library.
``HGGC::acfft``
  FFT library.
``HGGC::acrand``
  Random number library.
``HGGC::acdnn``
  DNN library.
``HGGC::acsparse``
  Sparse matrix library.
``HGGC::acsolver``
  Linear solver library.
``HGGC::hgjpeg``
  JPEG decode/encode library.
``HGGC::pccl``
  Collective communication library.

Result Variables
^^^^^^^^^^^^^^^^

``HGGCToolkit_FOUND``
``HGGCToolkit_VERSION``
``HGGCToolkit_ROOT``
``HGGCToolkit_INCLUDE_DIR``
``HGGCToolkit_LIBRARY_DIR``
``HGGCToolkit_BIN_DIR``

#]=======================================================================]

# --- Determine target platform triplet ---
if(CMAKE_SYSTEM_PROCESSOR)
  set(_hggc_target_dir "targets/${CMAKE_SYSTEM_PROCESSOR}-linux")
else()
  set(_hggc_target_dir "targets/x86_64-linux")
endif()

# --- Locate SDK root ---
set(_hggc_search_paths "")

# 1. HGGCToolkit_ROOT (cmake variable takes priority)
if(DEFINED HGGCToolkit_ROOT AND EXISTS "${HGGCToolkit_ROOT}")
  list(APPEND _hggc_search_paths "${HGGCToolkit_ROOT}")
endif()
if(DEFINED ENV{HGGCToolkit_ROOT} AND EXISTS "$ENV{HGGCToolkit_ROOT}")
  list(APPEND _hggc_search_paths "$ENV{HGGCToolkit_ROOT}")
endif()

# 2. CMAKE_HG_TOOLKIT_ROOT (set by enable_language(HG))
if(CMAKE_HG_TOOLKIT_ROOT)
  list(APPEND _hggc_search_paths "${CMAKE_HG_TOOLKIT_ROOT}")
endif()

# 3. PPU_SDK environment variable
if(DEFINED ENV{PPU_SDK})
  list(APPEND _hggc_search_paths "$ENV{PPU_SDK}")
endif()

# 4. Standard install locations
list(APPEND _hggc_search_paths
  "/usr/local/PPU_SDK")

# 5. If HG language is not enabled, try to find hgcc and derive from it
if(NOT CMAKE_HG_TOOLKIT_ROOT AND NOT HGGCToolkit_ROOT)
  find_program(_hggc_find_hgcc NAMES hgcc)
  if(_hggc_find_hgcc)
    get_filename_component(_hggc_from_bin "${_hggc_find_hgcc}" DIRECTORY)
    get_filename_component(_hggc_from_bin "${_hggc_from_bin}" DIRECTORY)
    list(APPEND _hggc_search_paths "${_hggc_from_bin}")
    unset(_hggc_from_bin)
  endif()
  unset(_hggc_find_hgcc CACHE)
endif()

# Find the root that contains the include directory
set(HGGCToolkit_ROOT "")
foreach(_path IN LISTS _hggc_search_paths)
  if(EXISTS "${_path}/${_hggc_target_dir}/include/hggc_runtime.h")
    set(HGGCToolkit_ROOT "${_path}")
    set(HGGCToolkit_INCLUDE_DIR "${_path}/${_hggc_target_dir}/include")
    set(HGGCToolkit_LIBRARY_DIR "${_path}/${_hggc_target_dir}/lib")
    break()
  elseif(EXISTS "${_path}/include/hggc_runtime.h")
    set(HGGCToolkit_ROOT "${_path}")
    set(HGGCToolkit_INCLUDE_DIR "${_path}/include")
    set(HGGCToolkit_LIBRARY_DIR "${_path}/lib")
    break()
  endif()
endforeach()
unset(_hggc_search_paths)

if(NOT HGGCToolkit_ROOT)
  if(HGGCToolkit_FIND_REQUIRED)
    message(FATAL_ERROR
      "Could not find HGGC Toolkit. Set HGGCToolkit_ROOT, CMAKE_HG_TOOLKIT_ROOT,\n"
      "PPU_SDK environment variable, or ensure the SDK is installed at /usr/local/PPU_SDK.")
  endif()
  set(HGGCToolkit_FOUND FALSE)
  return()
endif()

set(HGGCToolkit_BIN_DIR "${HGGCToolkit_ROOT}/bin")
set(_hggc_lib_search_dirs "${HGGCToolkit_LIBRARY_DIR}" "${HGGCToolkit_ROOT}/lib")

# --- Read version ---
set(HGGCToolkit_VERSION "")
if(CMAKE_HG_COMPILER_VERSION AND NOT CMAKE_HG_COMPILER_VERSION STREQUAL "0.0.0")
  set(HGGCToolkit_VERSION "${CMAKE_HG_COMPILER_VERSION}")
else()
  find_program(_hggc_compiler_for_ver NAMES hgcc
    HINTS "${HGGCToolkit_BIN_DIR}" NO_DEFAULT_PATH)
  if(_hggc_compiler_for_ver)
    execute_process(COMMAND "${_hggc_compiler_for_ver}" --version
      OUTPUT_VARIABLE _hggc_ver_out ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "Release version ([0-9]+\\.[0-9]+\\.[0-9]+[-a-zA-Z0-9]*)"
      _hggc_ver_match "${_hggc_ver_out}")
    if(_hggc_ver_match)
      set(HGGCToolkit_VERSION "${CMAKE_MATCH_1}")
    endif()
  endif()
  unset(_hggc_compiler_for_ver CACHE)

  if(NOT HGGCToolkit_VERSION AND EXISTS "${HGGCToolkit_ROOT}/VERSION.txt")
    file(READ "${HGGCToolkit_ROOT}/VERSION.txt" _hggc_ver_content)
    string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+[-a-zA-Z0-9]*)" _hggc_ver_match "${_hggc_ver_content}")
    if(_hggc_ver_match)
      set(HGGCToolkit_VERSION "${CMAKE_MATCH_1}")
    else()
      string(STRIP "${_hggc_ver_content}" HGGCToolkit_VERSION)
      string(REGEX REPLACE "\n.*" "" HGGCToolkit_VERSION "${HGGCToolkit_VERSION}")
    endif()
  endif()
endif()

# --- Helper: create an imported shared library target ---
function(_hggc_find_and_create_target _target_name _lib_name _header_name)
  if(NOT TARGET HGGC::${_target_name})
    set(_cache_var "_hggc_lib_${_target_name}")
    find_library(${_cache_var}
      NAMES "${_lib_name}"
      PATHS ${_hggc_lib_search_dirs}
      NO_DEFAULT_PATH)

    if(${_cache_var})
      add_library(HGGC::${_target_name} SHARED IMPORTED GLOBAL)
      set_target_properties(HGGC::${_target_name} PROPERTIES
        IMPORTED_LOCATION "${${_cache_var}}"
        IMPORTED_NO_SONAME TRUE)

      if(NOT "${_header_name}" STREQUAL "")
        set_target_properties(HGGC::${_target_name} PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${HGGCToolkit_INCLUDE_DIR}")
      endif()
    endif()
    unset(${_cache_var} CACHE)
  endif()
endfunction()

# --- Helper: create an imported static library target ---
function(_hggc_find_and_create_static_target _target_name _lib_name)
  if(NOT TARGET HGGC::${_target_name})
    set(_cache_var "_hggc_lib_${_target_name}")
    find_library(${_cache_var}
      NAMES "${_lib_name}"
      PATHS ${_hggc_lib_search_dirs}
      NO_DEFAULT_PATH)

    if(${_cache_var})
      add_library(HGGC::${_target_name} STATIC IMPORTED GLOBAL)
      set_target_properties(HGGC::${_target_name} PROPERTIES
        IMPORTED_LOCATION "${${_cache_var}}")
    endif()
    unset(${_cache_var} CACHE)
  endif()
endfunction()

# --- Create imported targets ---

# Core runtime
_hggc_find_and_create_target(hggcrt         hggcrt1          "hggc_runtime.h")
_hggc_find_and_create_target(hggc           hggc             "")

# Runtime compilation
_hggc_find_and_create_target(hgrtc          hgrtc            "hgrtc.h")
_hggc_find_and_create_target(hgrtc_builtins hgrtc_builtins   "")

# VM and JIT
_hggc_find_and_create_target(hgvm           hgvm             "hgvm.h")
_hggc_find_and_create_target(hgJitLink      hgJitLink        "hgJitLink.h")
_hggc_find_and_create_static_target(hgJitLink_static hgJitLink_static)

# Math libraries
_hggc_find_and_create_target(acblas         acblas            "acblas.h")
_hggc_find_and_create_target(acblasLt       acblasLt          "acblasLt.h")
_hggc_find_and_create_target(acfft          acfft             "acfft.h")
_hggc_find_and_create_target(acrand         acrand            "acrand.h")
_hggc_find_and_create_target(acsparse       acsparse          "acsparse.h")
_hggc_find_and_create_target(acsolver       acsolver          "acsolverDn.h")

# DNN
_hggc_find_and_create_target(acdnn          acdnn             "acdnn.h")

# Image codec
_hggc_find_and_create_target(hgjpeg         hgjpeg            "hgjpeg.h")

# Collective communication
_hggc_find_and_create_target(pccl           pccl              "pccl.h")

# --- Umbrella target: HGGC::toolkit ---
if(NOT TARGET HGGC::toolkit)
  add_library(HGGC::toolkit INTERFACE IMPORTED)
  set(_hggc_toolkit_deps "")
  foreach(_dep hggcrt hggc)
    if(TARGET HGGC::${_dep})
      list(APPEND _hggc_toolkit_deps "HGGC::${_dep}")
    endif()
  endforeach()
  if(_hggc_toolkit_deps)
    set_target_properties(HGGC::toolkit PROPERTIES
      INTERFACE_LINK_LIBRARIES "${_hggc_toolkit_deps}")
  endif()
  if(HGGCToolkit_INCLUDE_DIR)
    set_target_properties(HGGC::toolkit PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${HGGCToolkit_INCLUDE_DIR}")
  endif()
  unset(_hggc_toolkit_deps)
endif()

# --- Standard find_package result handling ---
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HGGCToolkit
  REQUIRED_VARS HGGCToolkit_ROOT HGGCToolkit_INCLUDE_DIR HGGCToolkit_LIBRARY_DIR
  VERSION_VAR   HGGCToolkit_VERSION)

mark_as_advanced(
  HGGCToolkit_ROOT
  HGGCToolkit_INCLUDE_DIR
  HGGCToolkit_LIBRARY_DIR
  HGGCToolkit_BIN_DIR)

unset(_hggc_target_dir)
unset(_hggc_lib_search_dirs)
