# Driver API + Fatbin mode
hgcc_add_fatbin(vector_add_kernel64 vector_add_kernel.hg
    INCLUDE_DIRECTORIES ${COMMON_DIR})

add_executable(driver_runtime_interop driver_runtime_interop.cpp)
target_include_directories(driver_runtime_interop PRIVATE ${COMMON_DIR})
target_link_libraries(driver_runtime_interop PRIVATE HGGC::toolkit)
