add_executable(mnist_cli)
target_sources(mnist_cli
    PUBLIC 
        src/main.cpp
)
target_link_libraries(mnist_cli PUBLIC nnet)
target_link_libraries(mnist_cli PUBLIC uvw::uvw)
