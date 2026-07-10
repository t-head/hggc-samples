add_executable(simple_acfft simple_acfft.hg)
target_include_directories(simple_acfft PRIVATE ${COMMON_DIR})
target_link_libraries(simple_acfft PRIVATE HGGC::acfft)
