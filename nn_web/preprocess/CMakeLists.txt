add_executable(preprocess)

target_link_libraries(preprocess PUBLIC nn_lib)
target_link_libraries(preprocess PUBLIC spdlog::spdlog)

target_sources(preprocess
    PRIVATE
        ./main.cpp
)
