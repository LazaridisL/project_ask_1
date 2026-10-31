cmake_minimum_required(VERSION 3.20)
project(imageSearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)

# Everything except main
add_library(search_core
    src/common.cpp
    src/sift_extractor.cpp
    src/kmeans.cpp
    src/dataset.cpp
    src/vocabulary.cpp
    src/options.cpp
    src/database.cpp
    src/exact_search.cpp
    src/metrics.cpp
    src/evaluation.cpp
    src/lsh_search.cpp
    src/ivfflat_search.cpp
    src/hypercube_search.cpp
    src/ivfpq_search.cpp
    src/cache.cpp
)
target_include_directories(search_core PUBLIC include)
target_link_libraries(search_core PUBLIC ${OpenCV_LIBS} OpenMP::OpenMP_CXX)
target_compile_options(search_core PRIVATE -Wall -Wextra -Wpedantic)

# The whole executable
add_executable(search src/main.cpp)
target_link_libraries(search PRIVATE search_core)
target_compile_options(search PRIVATE -Wall -Wextra -Wpedantic)