clean:
    rm -rf .cache build

setup:
    CC=clang CXX=clang++ cmake -G Ninja -S . -B build

build:
    cmake --build build

build-release:
    cmake --build build --config Release

run_nn_test_exe: build
    ./build/nn_lib/test

run_xor: build
    ./build/nn_lib/xor

run_mnist_view: build-release
    cd build/mnist && ./cli_view

run_mnist_train: build-release
    cd build/mnist && ./mnist_train
