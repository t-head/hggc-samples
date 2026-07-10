cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(hggc-samples LANGUAGES CXX HG)

find_package(HGGCToolkit REQUIRED)
include(HGGCUtilities)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

add_subdirectory(Samples)
