add_library(nn_lib STATIC)
target_sources(nn_lib 
    PUBLIC
        src/nn.cpp
        src/layer.cpp
)
target_include_directories(nn_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(nn_lib PUBLIC Eigen3::Eigen)

add_executable(test test/test.cpp)
target_link_libraries(test PRIVATE nn_lib)

add_executable(xor test/xor.cpp)
target_link_libraries(xor PRIVATE nn_lib)
