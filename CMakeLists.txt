cmake_minimum_required(VERSION 3.20)
project(imageSearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)

# Everything except main
add_library(search_core
    src/common.cpp
    src/sift_extractor.cpp
    src/kmeans.cpp
    src/vector_io.cpp
    src/dataset.cpp
)
target_include_directories(search_core PUBLIC include)
target_link_libraries(search_core PUBLIC ${OpenCV_LIBS})
target_compile_options(search_core PRIVATE -Wall -Wextra -Wpedantic)

# Tests
enable_testing()

add_executable(test_kmeans tests/test_kmeans.cpp)
target_link_libraries(test_kmeans PRIVATE search_core)
target_compile_options(test_kmeans PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME kmeans COMMAND test_kmeans)

# The whole executable
add_executable(search src/main.cpp)
target_link_libraries(search PRIVATE search_core)
target_compile_options(search PRIVATE -Wall -Wextra -Wpedantic)