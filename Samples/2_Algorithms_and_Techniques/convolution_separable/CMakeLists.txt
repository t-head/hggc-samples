# Separable convolution filter (mixed .hg + .cpp)
add_executable(convolution_separable
    convolutionSeparable.hg
    convolutionSeparable_gold.cpp
    main.cpp)
target_include_directories(convolution_separable PRIVATE ${COMMON_DIR})
target_link_libraries(convolution_separable PRIVATE HGGC::toolkit)
