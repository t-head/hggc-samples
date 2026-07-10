# Multi-context lifecycle management (Driver API + HGRTC + std::thread)
add_executable(ctx_management ctx_management.cpp)
target_include_directories(ctx_management PRIVATE ${COMMON_DIR})
target_link_libraries(ctx_management PRIVATE HGGC::hgrtc HGGC::toolkit)
