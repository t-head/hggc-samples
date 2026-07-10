add_executable(acdnn_conv_activation
    acdnn_conv_activation.cpp)
set_source_files_properties(
    acdnn_conv_activation.cpp
    PROPERTIES LANGUAGE HG)
target_include_directories(acdnn_conv_activation PRIVATE ${COMMON_DIR})
target_link_libraries(acdnn_conv_activation PRIVATE HGGC::toolkit HGGC::acdnn)
