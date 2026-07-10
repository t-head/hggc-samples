# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# Test that the HG (hgcc) compiler can produce a working object file.
# Also validates that the host compiler is compatible.

include(CMakeTestCompilerCommon)

if(CMAKE_HG_COMPILER_FORCED)
  set(CMAKE_HG_COMPILER_WORKS TRUE)
  return()
endif()

message(STATUS "Check for working HG compiler: ${CMAKE_HG_COMPILER}")

set(_test_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
set(_test_src "${_test_dir}/test_hgcc.hg")
file(WRITE "${_test_src}"
  "__global__ void _cmake_test_kernel() {}\n"
  "int main() { return 0; }\n")

# Build the compiler command with host compiler forwarding
set(_test_cmd "${CMAKE_HG_COMPILER}" -x hg)
if(CMAKE_HG_HOST_COMPILER)
  list(APPEND _test_cmd "-ccbin=${CMAKE_HG_HOST_COMPILER}")
endif()
list(APPEND _test_cmd -c "${_test_src}" -o "${_test_src}.o")

execute_process(
  COMMAND ${_test_cmd}
  RESULT_VARIABLE _hgcc_test_result
  OUTPUT_VARIABLE _hgcc_test_output
  ERROR_VARIABLE  _hgcc_test_error)

if(_hgcc_test_result EQUAL 0)
  set(CMAKE_HG_COMPILER_WORKS TRUE)
  message(STATUS "Check for working HG compiler: ${CMAKE_HG_COMPILER} - works")
  file(REMOVE "${_test_src}" "${_test_src}.o")
else()
  set(CMAKE_HG_COMPILER_WORKS FALSE)
  message(FATAL_ERROR
    "The HG compiler\n"
    "  ${CMAKE_HG_COMPILER}\n"
    "is not able to compile a simple test program.\n\n"
    "Command:\n  ${_test_cmd}\n\n"
    "Output:\n${_hgcc_test_output}\n${_hgcc_test_error}\n\n"
    "Check that the T-HEAD SAIL SDK is properly installed and that hgcc is compatible\n"
    "with the host compiler: ${CMAKE_HG_HOST_COMPILER}")
endif()

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/CMakeHGCompiler.cmake.in"
  "${CMAKE_PLATFORM_INFO_DIR}/CMakeHGCompiler.cmake"
  @ONLY)
