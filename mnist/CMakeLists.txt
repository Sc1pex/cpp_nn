add_library(mnist STATIC)
target_sources(mnist
    PUBLIC
        src/idx.cpp
)
target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mnist PUBLIC nn_lib)

add_executable(cli_view src/bin/cli_view.cpp)
target_link_libraries(cli_view PRIVATE mnist)
