clean:
    rm -rf .cache build

setup:
    CC=clang CXX=clang++ cmake -G Ninja -S . -B build

build:
    cmake --build build

build_release:
    cmake --build build --config Release

run_xor: build_release
    ./build/xor_test
