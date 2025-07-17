add_library(nn_lib STATIC)
target_sources(nn_lib 
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/matrix.cpp
)

target_include_directories(nn_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test test/test.cpp)
target_link_libraries(test PRIVATE nn_lib)
