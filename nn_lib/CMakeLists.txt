add_library(nn_lib STATIC
    src/matrix.cpp
)

target_include_directories(nn_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
