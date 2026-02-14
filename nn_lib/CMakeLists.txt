add_library(nn_lib)

target_compile_features(nn_lib PUBLIC cxx_std_23)

target_link_libraries(nn_lib PUBLIC Eigen3::Eigen)

target_sources(nn_lib
    PRIVATE
        ./src/activation.cpp
        ./src/network.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}/nn_lib/include
        FILES
            ${PROJECT_SOURCE_DIR}/nn_lib/include/nn_lib/activation.hpp
            ${PROJECT_SOURCE_DIR}/nn_lib/include/nn_lib/network.hpp
)
