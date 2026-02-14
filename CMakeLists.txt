cmake_minimum_required(VERSION 3.28)

project(
    mnist
    VERSION 0.1.0
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen/
    GIT_TAG        5.0.1
)
FetchContent_MakeAvailable(eigen)

FetchContent_Declare(
    httc
    GIT_REPOSITORY https://github.com/Sc1pex/httc
    GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(httc)

FetchContent_Declare(
    SQLite3
    GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake
    GIT_TAG        v3.50.2
)
FetchContent_MakeAvailable(SQLite3)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.17.0
)
set(SPDLOG_USE_STD_FORMAT ON)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(nn_lib)
add_subdirectory(nn_web/server)
