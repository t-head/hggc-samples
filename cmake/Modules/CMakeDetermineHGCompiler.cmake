# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# Determine the HG (HGGC) compiler to use.
#
# This module is loaded by `enable_language(HG)` or `project(... LANGUAGES HG)`.
# It finds hgcc, extracts its version, derives the SDK root, determines the
# host link launcher, and extracts implicit link information — mirroring the
# approach CMake uses for language compiler detection.
#
# Search order for hgcc:
#   1. CMAKE_HG_COMPILER (user-specified)
#   2. HGCC environment variable
#   3. $ENV{PPU_SDK}/bin/hgcc
#   4. PATH search for "hgcc"

set(CMAKE_HG_COMPILER_ENV_VAR "HGCC")

if(NOT CMAKE_HG_COMPILER)
  if(DEFINED ENV{HGCC} AND NOT "$ENV{HGCC}" STREQUAL "")
    get_filename_component(CMAKE_HG_COMPILER "$ENV{HGCC}" PROGRAM)
    if(NOT EXISTS "${CMAKE_HG_COMPILER}")
      message(FATAL_ERROR
        "Could not find hgcc compiler set in environment variable HGCC:\n"
        "  $ENV{HGCC}")
    endif()
  endif()

  if(NOT CMAKE_HG_COMPILER AND DEFINED ENV{PPU_SDK})
    set(_hgcc_sdk_candidate "$ENV{PPU_SDK}/bin/hgcc")
    if(EXISTS "${_hgcc_sdk_candidate}")
      set(CMAKE_HG_COMPILER "${_hgcc_sdk_candidate}")
    endif()
    unset(_hgcc_sdk_candidate)
  endif()

  if(NOT CMAKE_HG_COMPILER)
    find_program(CMAKE_HG_COMPILER NAMES hgcc)
  endif()
endif()

if(NOT CMAKE_HG_COMPILER)
  message(FATAL_ERROR
    "Could not find hgcc compiler. Set CMAKE_HG_COMPILER, the HGCC environment\n"
    "variable, or ensure hgcc is in PATH. If using T-HEAD SAIL SDK, source envsetup.sh first.")
endif()

mark_as_advanced(CMAKE_HG_COMPILER)

# --- Extract compiler version ---
if(NOT CMAKE_HG_COMPILER_VERSION)
  execute_process(
    COMMAND "${CMAKE_HG_COMPILER}" --version
    OUTPUT_VARIABLE _hgcc_version_output
    ERROR_VARIABLE  _hgcc_version_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _hgcc_version_result)

  if(_hgcc_version_result EQUAL 0)
    string(REGEX MATCH "Release version ([0-9]+\\.[0-9]+\\.[0-9]+[-a-zA-Z0-9]*)"
      _hgcc_ver_match "${_hgcc_version_output}")
    if(_hgcc_ver_match)
      set(CMAKE_HG_COMPILER_VERSION "${CMAKE_MATCH_1}")
    endif()
  endif()

  if(NOT CMAKE_HG_COMPILER_VERSION)
    set(CMAKE_HG_COMPILER_VERSION "0.0.0")
    message(WARNING "Could not determine hgcc version, defaulting to 0.0.0")
  endif()
endif()

set(CMAKE_HG_COMPILER_ID "HGCC")
set(CMAKE_HG_COMPILER_LOADED TRUE)

# --- Derive toolkit root from compiler path ---
if(NOT CMAKE_HG_TOOLKIT_ROOT)
  get_filename_component(_hgcc_bin_dir "${CMAKE_HG_COMPILER}" DIRECTORY)
  get_filename_component(CMAKE_HG_TOOLKIT_ROOT "${_hgcc_bin_dir}" DIRECTORY)
  unset(_hgcc_bin_dir)
endif()

# --- Determine host compiler ---
if(NOT CMAKE_HG_HOST_COMPILER)
  if(DEFINED ENV{HGCCHOSTCXX} AND NOT "$ENV{HGCCHOSTCXX}" STREQUAL "")
    set(CMAKE_HG_HOST_COMPILER "$ENV{HGCCHOSTCXX}")
  elseif(CMAKE_CXX_COMPILER)
    set(CMAKE_HG_HOST_COMPILER "${CMAKE_CXX_COMPILER}")
  else()
    find_program(CMAKE_HG_HOST_COMPILER NAMES g++ c++)
  endif()
endif()

# --- Extract implicit link information ---
# Run hgcc -v on a dummy link to discover host linker, implicit libraries, and
# implicit link directories — exactly how CMake discovers this for other language compilers.
if(NOT DEFINED CMAKE_HG_HOST_LINK_LAUNCHER)
  set(_hg_test_src "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/hg_implicit_link.hg")
  set(_hg_test_obj "${_hg_test_src}.o")
  set(_hg_test_out "${_hg_test_src}.out")

  file(WRITE "${_hg_test_src}" "int main(){return 0;}\n")

  execute_process(
    COMMAND "${CMAKE_HG_COMPILER}" -x hg -c "${_hg_test_src}" -o "${_hg_test_obj}"
    RESULT_VARIABLE _hg_compile_result
    OUTPUT_QUIET ERROR_QUIET)

  if(_hg_compile_result EQUAL 0)
    execute_process(
      COMMAND "${CMAKE_HG_COMPILER}" -v "${_hg_test_obj}" -o "${_hg_test_out}"
      OUTPUT_VARIABLE _hg_link_verbose
      ERROR_VARIABLE  _hg_link_verbose
      RESULT_VARIABLE _hg_link_result)
  endif()

  file(REMOVE "${_hg_test_src}" "${_hg_test_obj}" "${_hg_test_out}")

  if(_hg_link_result EQUAL 0 AND _hg_link_verbose)
    # Extract the host link line from hgcc -v output.
    # The line starts with "#$ <host-compiler>" and contains "-lhggcrt1".
    # We match any host compiler (g++, clang++, c++, etc.), not just g++.
    string(REPLACE "\n" ";" _hg_link_lines "${_hg_link_verbose}")
    set(_hg_final_link_line "")
    foreach(_line IN LISTS _hg_link_lines)
      if(_line MATCHES "^#\\$ " AND _line MATCHES "-lhggcrt1")
        string(REGEX REPLACE "^#\\$ " "" _hg_final_link_line "${_line}")
      endif()
    endforeach()

    if(_hg_final_link_line)
      # Tokenize the link line and classify each token
      separate_arguments(_hg_link_tokens UNIX_COMMAND "${_hg_final_link_line}")

      # First token is the host link launcher
      list(GET _hg_link_tokens 0 _hg_link_launcher)
      if(_hg_link_launcher)
        find_program(CMAKE_HG_HOST_LINK_LAUNCHER NAMES "${_hg_link_launcher}")
        if(NOT CMAKE_HG_HOST_LINK_LAUNCHER)
          set(CMAKE_HG_HOST_LINK_LAUNCHER "${_hg_link_launcher}")
        endif()
      endif()

      set(CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES "")
      set(CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES "")

      foreach(_token IN LISTS _hg_link_tokens)
        if(_token MATCHES "^-L(.+)")
          list(APPEND CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES "${CMAKE_MATCH_1}")
        elseif(_token MATCHES "^-l(.+)")
          list(APPEND CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES "${CMAKE_MATCH_1}")
        endif()
      endforeach()
    endif()
  endif()

  # Fallbacks if extraction failed
  if(NOT CMAKE_HG_HOST_LINK_LAUNCHER)
    set(CMAKE_HG_HOST_LINK_LAUNCHER "${CMAKE_HG_HOST_COMPILER}")
  endif()
  if(NOT CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES)
    set(CMAKE_HG_HOST_IMPLICIT_LINK_LIBRARIES "hggc;hggcrt1")
  endif()
  if(NOT CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES)
    set(CMAKE_HG_HOST_IMPLICIT_LINK_DIRECTORIES
      "${CMAKE_HG_TOOLKIT_ROOT}/targets/${CMAKE_SYSTEM_PROCESSOR}-linux/lib")
  endif()
endif()

# --- Toolkit include directories ---
if(NOT CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES)
  set(CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES "")
  set(_hg_target_inc "${CMAKE_HG_TOOLKIT_ROOT}/targets/${CMAKE_SYSTEM_PROCESSOR}-linux/include")
  if(IS_DIRECTORY "${_hg_target_inc}")
    list(APPEND CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES "${_hg_target_inc}")
  endif()
  set(_hg_root_inc "${CMAKE_HG_TOOLKIT_ROOT}/include")
  if(IS_DIRECTORY "${_hg_root_inc}")
    list(APPEND CMAKE_HG_TOOLKIT_INCLUDE_DIRECTORIES "${_hg_root_inc}")
  endif()
  unset(_hg_target_inc)
  unset(_hg_root_inc)
endif()

# --- Device linker ---
set(CMAKE_HG_DEVICE_LINKER "${CMAKE_HG_TOOLKIT_ROOT}/bin/hglink")

# --- List supported architectures ---
if(NOT CMAKE_HG_KNOWN_ARCHITECTURES)
  execute_process(
    COMMAND "${CMAKE_HG_COMPILER}" --list-gpu-arch
    OUTPUT_VARIABLE _hgcc_arch_output
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _hgcc_arch_result)

  if(_hgcc_arch_result EQUAL 0 AND _hgcc_arch_output)
    string(REPLACE "\n" ";" CMAKE_HG_KNOWN_ARCHITECTURES "${_hgcc_arch_output}")
  else()
    set(CMAKE_HG_KNOWN_ARCHITECTURES "ppu_10;ppu_15;vm_10;vm_15")
  endif()
endif()

message(STATUS "Found HG compiler: ${CMAKE_HG_COMPILER} (version ${CMAKE_HG_COMPILER_VERSION})")
message(STATUS "HG toolkit root: ${CMAKE_HG_TOOLKIT_ROOT}")
message(STATUS "HG host link launcher: ${CMAKE_HG_HOST_LINK_LAUNCHER}")

# --- Persist to build directory ---
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/CMakeHGCompiler.cmake.in"
  "${CMAKE_PLATFORM_INFO_DIR}/CMakeHGCompiler.cmake"
  @ONLY)
