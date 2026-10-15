cmake_minimum_required(VERSION 3.20)

get_filename_component(HGGC_SAMPLES_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(HGGC_CI_CASES_FILE "${CMAKE_CURRENT_LIST_DIR}/cases.json")
file(READ "${HGGC_CI_CASES_FILE}" HGGC_CI_CASES_JSON)

string(JSON HGGC_CI_SCHEMA_VERSION GET "${HGGC_CI_CASES_JSON}" schema_version)
if(NOT HGGC_CI_SCHEMA_VERSION EQUAL 1)
  message(FATAL_ERROR "Unsupported cases.json schema: ${HGGC_CI_SCHEMA_VERSION}")
endif()

string(JSON HGGC_CI_ARCHITECTURE GET "${HGGC_CI_CASES_JSON}" architecture)
if(NOT CMAKE_HG_ARCHITECTURES)
  set(CMAKE_HG_ARCHITECTURES "${HGGC_CI_ARCHITECTURE}" CACHE STRING
      "Target HG device architecture for the CI device pool" FORCE)
endif()
if(NOT CMAKE_HG_ARCHITECTURES STREQUAL HGGC_CI_ARCHITECTURE)
  message(FATAL_ERROR
      "CI cases target ${HGGC_CI_ARCHITECTURE}, but CMAKE_HG_ARCHITECTURES="
      "${CMAKE_HG_ARCHITECTURES}")
endif()

list(APPEND CMAKE_MODULE_PATH "${HGGC_SAMPLES_ROOT}/cmake/Modules")
project(hggc-samples-ci LANGUAGES CXX HG)

find_package(HGGCToolkit REQUIRED)
include(HGGCUtilities)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(COMMON_DIR "${HGGC_SAMPLES_ROOT}/Common")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

string(JSON HGGC_CI_SAMPLE_COUNT LENGTH "${HGGC_CI_CASES_JSON}" samples)
if(HGGC_CI_SAMPLE_COUNT LESS 1)
  message(FATAL_ERROR "cases.json must select at least one sample directory")
endif()

set(HGGC_CI_TARGETS)
set(HGGC_CI_MANIFEST "")
math(EXPR HGGC_CI_LAST_SAMPLE "${HGGC_CI_SAMPLE_COUNT} - 1")

foreach(HGGC_CI_SAMPLE_INDEX RANGE 0 ${HGGC_CI_LAST_SAMPLE})
  string(JSON HGGC_CI_SOURCE GET
      "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} source)
  string(JSON HGGC_CI_BINARY_DIR GET
      "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} binary_dir)

  set(HGGC_CI_SOURCE_DIR "${HGGC_SAMPLES_ROOT}/${HGGC_CI_SOURCE}")
  if(NOT EXISTS "${HGGC_CI_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Selected sample has no CMakeLists.txt: ${HGGC_CI_SOURCE}")
  endif()

  # EXCLUDE_FROM_ALL and the explicit aggregate target below ensure that new
  # targets added to a selected sample do not silently expand CI coverage.
  add_subdirectory(
      "${HGGC_CI_SOURCE_DIR}"
      "${CMAKE_BINARY_DIR}/samples/${HGGC_CI_BINARY_DIR}"
      EXCLUDE_FROM_ALL)

  string(JSON HGGC_CI_CASE_COUNT LENGTH
      "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases)
  if(HGGC_CI_CASE_COUNT LESS 1)
    message(FATAL_ERROR "Selected sample has no cases: ${HGGC_CI_SOURCE}")
  endif()
  math(EXPR HGGC_CI_LAST_CASE "${HGGC_CI_CASE_COUNT} - 1")

  foreach(HGGC_CI_CASE_INDEX RANGE 0 ${HGGC_CI_LAST_CASE})
    string(JSON HGGC_CI_TARGET GET
        "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases
        ${HGGC_CI_CASE_INDEX} target)
    string(JSON HGGC_CI_SUCCESS GET
        "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases
        ${HGGC_CI_CASE_INDEX} success)
    string(JSON HGGC_CI_DESCRIPTION GET
        "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases
        ${HGGC_CI_CASE_INDEX} description)

    if(NOT TARGET "${HGGC_CI_TARGET}")
      message(FATAL_ERROR
          "Selected target '${HGGC_CI_TARGET}' is not defined by ${HGGC_CI_SOURCE}")
    endif()
    if(HGGC_CI_TARGET IN_LIST HGGC_CI_TARGETS)
      message(FATAL_ERROR "Duplicate selected target: ${HGGC_CI_TARGET}")
    endif()
    list(APPEND HGGC_CI_TARGETS "${HGGC_CI_TARGET}")

    string(JSON HGGC_CI_ASSET_COUNT LENGTH
        "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases
        ${HGGC_CI_CASE_INDEX} assets)
    set(HGGC_CI_ASSETS "")
    if(HGGC_CI_ASSET_COUNT GREATER 0)
      math(EXPR HGGC_CI_LAST_ASSET "${HGGC_CI_ASSET_COUNT} - 1")
      foreach(HGGC_CI_ASSET_INDEX RANGE 0 ${HGGC_CI_LAST_ASSET})
        string(JSON HGGC_CI_ASSET GET
            "${HGGC_CI_CASES_JSON}" samples ${HGGC_CI_SAMPLE_INDEX} cases
            ${HGGC_CI_CASE_INDEX} assets ${HGGC_CI_ASSET_INDEX})
        if(HGGC_CI_ASSETS)
          string(APPEND HGGC_CI_ASSETS ",")
        endif()
        string(APPEND HGGC_CI_ASSETS "${HGGC_CI_ASSET}")
      endforeach()
    endif()

    string(APPEND HGGC_CI_MANIFEST
        "$<TARGET_FILE_NAME:${HGGC_CI_TARGET}>\t${HGGC_CI_SUCCESS}\t"
        "${HGGC_CI_DESCRIPTION}\t${HGGC_CI_ASSETS}\n")
  endforeach()
endforeach()

add_custom_target(hggc_ci_cases DEPENDS ${HGGC_CI_TARGETS})
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/ci-cases.tsv"
    CONTENT "${HGGC_CI_MANIFEST}")

list(LENGTH HGGC_CI_TARGETS HGGC_CI_TARGET_COUNT)
message(STATUS
    "Selected ${HGGC_CI_TARGET_COUNT} allowlisted CI cases for ${HGGC_CI_ARCHITECTURE}")
