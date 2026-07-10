add_executable(hg_jpeg hg_jpeg.cpp)
set_source_files_properties(hg_jpeg.cpp PROPERTIES LANGUAGE HG)
target_include_directories(hg_jpeg PRIVATE ${COMMON_DIR})
target_link_libraries(hg_jpeg PRIVATE HGGC::toolkit HGGC::hgjpeg)

# Copy data to the output directory
add_custom_command(TARGET hg_jpeg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/images
        $<TARGET_FILE_DIR:hg_jpeg>/images)
