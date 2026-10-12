cmake_minimum_required(VERSION 3.20)
project(ImageSearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# add_executable()

target_link_libraries(imageSearch PRIVATE
    ${OpenCV_LIBS}
    Eigen3::Eigen
)