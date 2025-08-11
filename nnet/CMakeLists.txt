add_library(nnet STATIC)
target_include_directories(nnet PUBLIC include)
target_sources(nnet 
    PUBLIC 
        src/nn.cpp
        src/idx.cpp
        src/train_pool.cpp
)
target_link_libraries(nnet PUBLIC Eigen3::Eigen)

add_executable(xor_test)
target_sources(xor_test
    PUBLIC 
        bin/xor_test.cpp
)
target_link_libraries(xor_test PUBLIC nnet)

add_executable(mnist_benchmark)
target_sources(mnist_benchmark
    PUBLIC 
        bin/mnist_benchmark.cpp
)
target_link_libraries(mnist_benchmark PUBLIC nnet)
