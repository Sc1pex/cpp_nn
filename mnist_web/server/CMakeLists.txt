add_executable(server)
target_include_directories(server PUBLIC include)
target_sources(server
    PUBLIC
        src/main.cpp
)
