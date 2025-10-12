add_executable(server)
target_include_directories(server PUBLIC include)
target_sources(server
    PUBLIC
        src/main.cpp
        src/app.cpp
        src/db.cpp
)
target_link_libraries(server PRIVATE httc::httc)
target_link_libraries(server PRIVATE asio::asio)
target_link_libraries(server PRIVATE unofficial::sqlite3::sqlite3)
