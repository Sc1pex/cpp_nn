cmake_minimum_required(VERSION 3.31)
project(Mnist)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Eigen3)

add_library(nnet STATIC)
target_include_directories(nnet PUBLIC nnet/include)
target_sources(nnet 
    PUBLIC 
)
target_link_libraries(nnet PUBLIC Eigen3::Eigen)
