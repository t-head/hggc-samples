# Copyright (c) 2026 T-Head (Shanghai) Semiconductor Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
#[=======================================================================[.rst:
HGGCUtilities
-------------

Convenience functions for HGGC development with CMake.

Functions
^^^^^^^^^

``hgcc_add_fatbin``
  Compile .hg source(s) to a device-only fatbin file.

``hgcc_enable_rdc``
  Enable RDC (relocatable device code) / separable compilation for a target.

``hgcc_add_rdc_executable``
  Create an executable with RDC enabled.

``hgcc_add_rdc_library``
  Create a library with RDC enabled.

``hgcc_set_standard``
  Set C++ standard for HG sources in a specific target.

``hgcc_fatbin_to_header``
  Convert a fatbin file to an embeddable C header using hgbin2c.

#]=======================================================================]


# hgcc_add_fatbin(<name> <source>
#   [ARCHITECTURES arch1 arch2 ...]
#   [INCLUDE_DIRECTORIES dir1 dir2 ...]
#   [DEPENDS dep1 dep2 ...])
#
# Compile a .hg source file into a device-only .fatbin file.
# Creates a custom target <name> and sets <name>_OUTPUT to the fatbin path.
function(hgcc_add_fatbin _name _source)
  cmake_parse_arguments(_arg "" "" "ARCHITECTURES;INCLUDE_DIRECTORIES;DEPENDS" ${ARGN})

  if(NOT CMAKE_HG_COMPILER)
    message(FATAL_ERROR "hgcc_add_fatbin: HG language not enabled. Call enable_language(HG) first.")
  endif()

  get_filename_component(_src_abs "${_source}" ABSOLUTE)
  set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_name}.fatbin")

  # Build flag list
  set(_flags -x hg --fatbin)

  # Architectures
  if(_arg_ARCHITECTURES)
    foreach(_arch IN LISTS _arg_ARCHITECTURES)
      list(APPEND _flags "--gpu-architecture=${_arch}")
    endforeach()
  elseif(DEFINED CMAKE_HG_ARCHITECTURES AND NOT CMAKE_HG_ARCHITECTURES STREQUAL "")
    foreach(_arch IN LISTS CMAKE_HG_ARCHITECTURES)
      list(APPEND _flags "--gpu-architecture=${_arch}")
    endforeach()
  endif()

  # Include directories
  if(_arg_INCLUDE_DIRECTORIES)
    foreach(_dir IN LISTS _arg_INCLUDE_DIRECTORIES)
      list(APPEND _flags "-I${_dir}")
    endforeach()
  endif()

  # Host compiler
  if(CMAKE_HG_HOST_COMPILER)
    list(APPEND _flags "--ccbin=${CMAKE_HG_HOST_COMPILER}")
  endif()

  set(_depends "${_src_abs}")
  if(_arg_DEPENDS)
    list(APPEND _depends ${_arg_DEPENDS})
  endif()

  add_custom_command(
    OUTPUT "${_output}"
    COMMAND "${CMAKE_HG_COMPILER}" ${_flags} "${_src_abs}" -o "${_output}"
    DEPENDS ${_depends}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building HGGC fatbin: ${_name}.fatbin"
    VERBATIM)

  add_custom_target(${_name} ALL DEPENDS "${_output}")

  set(${_name}_OUTPUT "${_output}" PARENT_SCOPE)
endfunction()


# hgcc_enable_rdc(<target>)
#
# Enable RDC (Relocatable Device Code) / separable compilation for a target.
# - Adds --device-c to HG compile options
# - Sets LINKER_LANGUAGE to HG so hgcc handles device linking automatically
#
function(hgcc_enable_rdc _target)
  if(NOT TARGET ${_target})
    message(FATAL_ERROR "hgcc_enable_rdc: '${_target}' is not a target.")
  endif()

  target_compile_options(${_target} PRIVATE
    $<$<COMPILE_LANGUAGE:HG>:--device-c>)
  set_target_properties(${_target} PROPERTIES
    HG_SEPARABLE_COMPILATION ON
    LINKER_LANGUAGE HG)
endfunction()


# hgcc_add_rdc_executable(<name> <sources...>)
#
# Create an executable with RDC (separable compilation) enabled.
# Supports mixed .hg + .cpp sources. hgcc is used as the linker to
# automatically handle the device link phase.
#
# Example:
#   hgcc_add_rdc_executable(myapp rdc_a.hg rdc_b.hg host.cpp)
function(hgcc_add_rdc_executable _name)
  add_executable(${_name} ${ARGN})
  hgcc_enable_rdc(${_name})
endfunction()


# hgcc_add_rdc_library(<name> <type> <sources...>)
#
# Create a library with RDC enabled.
# <type> is STATIC, SHARED, or MODULE.
#
# Example:
#   hgcc_add_rdc_library(mylib STATIC rdc_a.hg rdc_b.hg)
function(hgcc_add_rdc_library _name)
  add_library(${_name} ${ARGN})
  hgcc_enable_rdc(${_name})
endfunction()


# hgcc_set_standard(<target> <standard>)
#
# Set C++ standard for HG sources in a specific target.
# Overrides the global CMAKE_HG_STANDARD for this target only.
#
# Example:
#   hgcc_set_standard(myapp 17)
function(hgcc_set_standard _target _std)
  if(NOT TARGET ${_target})
    message(FATAL_ERROR "hgcc_set_standard: '${_target}' is not a target.")
  endif()
  if(NOT DEFINED CMAKE_HG${_std}_STANDARD_COMPILE_OPTION)
    message(FATAL_ERROR "hgcc_set_standard: C++${_std} is not supported by hgcc. "
      "Supported values: 11, 14, 17, 20")
  endif()
  target_compile_options(${_target} PRIVATE
    $<$<COMPILE_LANGUAGE:HG>:${CMAKE_HG${_std}_STANDARD_COMPILE_OPTION}>)
endfunction()


# hgcc_fatbin_to_header(<fatbin_file> <output_header>)
#
# Convert a fatbin binary to a C header file using hgbin2c.
# The header contains a const array with the fatbin data.
function(hgcc_fatbin_to_header _fatbin _header)
  if(NOT CMAKE_HG_TOOLKIT_ROOT)
    message(FATAL_ERROR "hgcc_fatbin_to_header: CMAKE_HG_TOOLKIT_ROOT not set.")
  endif()

  find_program(_hgbin2c hgbin2c
    HINTS "${CMAKE_HG_TOOLKIT_ROOT}/bin"
    NO_DEFAULT_PATH)

  if(NOT _hgbin2c)
    find_program(_hgbin2c hgbin2c)
  endif()

  if(NOT _hgbin2c)
    message(FATAL_ERROR "hgcc_fatbin_to_header: Could not find hgbin2c tool.")
  endif()

  get_filename_component(_fatbin_abs "${_fatbin}" ABSOLUTE)
  get_filename_component(_header_abs "${_header}" ABSOLUTE)

  # hgbin2c writes to stdout; use shell redirection via sh -c
  add_custom_command(
    OUTPUT "${_header_abs}"
    COMMAND sh -c "\"${_hgbin2c}\" -p 0 \"${_fatbin_abs}\" > \"${_header_abs}\""
    DEPENDS "${_fatbin_abs}"
    COMMENT "Generating header from fatbin: ${_header}"
    VERBATIM)

  mark_as_advanced(_hgbin2c)
endfunction()
