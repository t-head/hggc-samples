# This layer of CMakeLists.txt adds folders, for better organization in Visual Studio
# and other IDEs that support this feature.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_FOLDER "0_Introduction")
add_subdirectory(0_Introduction)

set(CMAKE_FOLDER "1_Utilities")
add_subdirectory(1_Utilities)

set(CMAKE_FOLDER "2_Algorithms_and_Techniques")
add_subdirectory(2_Algorithms_and_Techniques)

set(CMAKE_FOLDER "3_HGGC_Features")
add_subdirectory(3_HGGC_Features)

set(CMAKE_FOLDER "4_HGGC_Libraries")
add_subdirectory(4_HGGC_Libraries)

set(CMAKE_FOLDER "5_Performance")
add_subdirectory(5_Performance)


