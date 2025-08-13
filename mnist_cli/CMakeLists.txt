add_executable(mnist_cli)
target_include_directories(mnist_cli PUBLIC include)
target_sources(mnist_cli
    PUBLIC 
        src/main.cpp
        src/app.cpp
        src/cli.cpp
        src/key.cpp
)
target_link_libraries(mnist_cli PUBLIC nnet)
target_link_libraries(mnist_cli PUBLIC uvw::uvw)
