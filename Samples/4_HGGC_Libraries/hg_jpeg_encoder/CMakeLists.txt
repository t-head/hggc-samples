add_executable(hg_jpeg_encoder hg_jpeg_encoder.cpp)
set_source_files_properties(hg_jpeg_encoder.cpp PROPERTIES LANGUAGE HG)
target_include_directories(hg_jpeg_encoder PRIVATE ${COMMON_DIR})
target_link_libraries(hg_jpeg_encoder PRIVATE HGGC::toolkit HGGC::hgjpeg)

# Copy data to the output directory
add_custom_command(TARGET hg_jpeg_encoder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/images
        $<TARGET_FILE_DIR:hg_jpeg_encoder>/images
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:hg_jpeg_encoder>/encode_output)
