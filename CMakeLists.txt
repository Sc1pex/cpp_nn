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
        nnet/src/nn.cpp
        nnet/src/idx.cpp
        nnet/src/train_pool.cpp
)
target_link_libraries(nnet PUBLIC Eigen3::Eigen)

add_executable(xor_test)
target_sources(xor_test
    PUBLIC 
        nnet/bin/xor_test.cpp
)
target_link_libraries(xor_test PUBLIC nnet)

add_executable(mnist_cli)
target_sources(mnist_cli
    PUBLIC 
        nnet/bin/mnist_cli.cpp
)
target_link_libraries(mnist_cli PUBLIC nnet)
