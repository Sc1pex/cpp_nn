add_library(mnist STATIC)
target_sources(mnist
    PUBLIC
        src/idx.cpp
)
target_include_directories(mnist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mnist PUBLIC nn_lib)

add_executable(cli_view src/bin/cli_view.cpp)
target_link_libraries(cli_view PRIVATE mnist)
add_custom_command(TARGET cli_view POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:cli_view>/data
    COMMENT "Copying assets to output directory"
)

add_executable(mnist_train src/bin/mnist_train.cpp)
target_link_libraries(mnist_train PRIVATE mnist)
add_custom_command(TARGET mnist_train POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:cli_view>/data
    COMMENT "Copying assets to output directory"
)
