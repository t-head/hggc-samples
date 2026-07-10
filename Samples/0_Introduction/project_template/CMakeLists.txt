# Mixed compilation: .hg kernel + .cpp host
add_executable(project_template project_template.hg project_template_cpu.cpp)
target_include_directories(project_template PRIVATE ${COMMON_DIR})
target_link_libraries(project_template PRIVATE HGGC::toolkit)
