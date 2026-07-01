add_executable(preprocess)

target_link_libraries(preprocess PUBLIC nn_lib)
target_link_libraries(preprocess PUBLIC spdlog::spdlog)

target_sources(preprocess
    PRIVATE
        ./src/idx.cpp
        ./src/cli.cpp
        ./src/main.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}/nn_web/preprocess/include
        FILES
            ${PROJECT_SOURCE_DIR}/nn_web/preprocess/include/idx.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/preprocess/include/cli.hpp
)
