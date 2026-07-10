# ---------------------------------------------------------------------------
# Target 1: device_query  (Runtime API)
# ---------------------------------------------------------------------------
add_executable(device_query device_query.cpp)
set_source_files_properties(device_query.cpp PROPERTIES LANGUAGE HG)
target_include_directories(device_query PRIVATE ${COMMON_DIR})
target_link_libraries(device_query PRIVATE HGGC::toolkit)

# ---------------------------------------------------------------------------
# Target 2: device_query_drv  (Driver API)
# ---------------------------------------------------------------------------
add_executable(device_query_drv device_query_drv.cpp)
set_source_files_properties(device_query_drv.cpp PROPERTIES LANGUAGE HG)
target_include_directories(device_query_drv PRIVATE ${COMMON_DIR})
target_link_libraries(device_query_drv PRIVATE HGGC::toolkit)
