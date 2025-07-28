add_library(nn_lib STATIC)
target_sources(nn_lib 
    PUBLIC
        src/matrix.cpp
        src/nn.cpp
        src/layer.cpp
)
target_include_directories(nn_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test test/test.cpp)
target_link_libraries(test PRIVATE nn_lib)

add_executable(xor test/xor.cpp)
target_link_libraries(xor PRIVATE nn_lib)
