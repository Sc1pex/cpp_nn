add_library(nn_lib STATIC)
target_sources(nn_lib 
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/matrix.cpp
            src/types.cpp
            src/lib.cpp
            src/nn.cpp
)

add_executable(test test/test.cpp)
target_link_libraries(test PRIVATE nn_lib)
