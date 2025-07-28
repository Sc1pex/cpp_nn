clean:
    rm -rf .cache build

setup:
    CC=clang CXX=clang++ cmake -G Ninja -S . -B build

build:
    cmake --build build

run_nn_test_exe: build
    ./build/nn_lib/test

run_xor: build
    ./build/nn_lib/xor

run_mnist_view: build
    cd build/mnist && ./cli_view
