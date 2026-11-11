add_executable(array_roundtrip array_roundtrip.hg)
target_include_directories(array_roundtrip PRIVATE ${COMMON_DIR})
target_link_libraries(array_roundtrip PRIVATE HGGC::toolkit)
