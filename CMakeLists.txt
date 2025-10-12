cmake_minimum_required(VERSION 3.31)
project(Mnist)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Eigen3 CONFIG REQUIRED)
find_package(httc CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
add_compile_definitions(EIGEN_NO_DEBUG)
add_compile_definitions(EIGEN_DONT_PARALLELIZE)

add_subdirectory(nnet)
add_subdirectory(mnist_web/server)
