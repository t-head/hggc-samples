add_executable(jit_lto jit_lto.cpp)
set_source_files_properties(jit_lto.cpp PROPERTIES LANGUAGE HG)
target_include_directories(jit_lto PRIVATE ${COMMON_DIR})
target_link_libraries(jit_lto PRIVATE HGGC::toolkit HGGC::hgrtc HGGC::hgJitLink)
