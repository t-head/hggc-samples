cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# Default to building for both ppu001 (ppu_10) and ppu0015 (ppu_15).
# Must be set BEFORE project() enables the HG language, otherwise
# CMakeHGInformation.cmake will not pick it up.
# Command-line -DCMAKE_HG_ARCHITECTURES=... still overrides this default.
if(NOT CMAKE_HG_ARCHITECTURES)
  set(CMAKE_HG_ARCHITECTURES "ppu_10;ppu_15" CACHE STRING
      "Target HG device architectures (semicolon-separated)" FORCE)
endif()

project(hggc-samples LANGUAGES CXX HG)

find_package(HGGCToolkit REQUIRED)
include(HGGCUtilities)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

add_subdirectory(Samples)
