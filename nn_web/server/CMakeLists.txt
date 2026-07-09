add_executable(nn_server)

target_link_libraries(nn_server PUBLIC nn_lib)
target_link_libraries(nn_server PUBLIC httc::httc)
target_link_libraries(nn_server PUBLIC SQLite::SQLite3)
target_link_libraries(nn_server PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(nn_server PUBLIC spdlog::spdlog)

target_sources(nn_server
    PRIVATE
        ./src/api_response.cpp
        ./src/app.cpp
        ./src/db.cpp
        ./src/db_statement.cpp
        ./src/main.cpp
        ./src/memmat.cpp
        ./src/state.cpp
        ./src/utils.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}/nn_web/server/include
        FILES
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/api_response.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/app.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/db.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/db_error.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/db_statement.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/memmat.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/state.hpp
            ${PROJECT_SOURCE_DIR}/nn_web/server/include/utils.hpp
)
