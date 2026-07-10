# 256-bin histogram with progressive optimization strategies
add_executable(histogram
    histogram.hg
    histogram_gold.cpp
    main.cpp)
target_include_directories(histogram PRIVATE ${COMMON_DIR})
target_link_libraries(histogram PRIVATE HGGC::toolkit)
