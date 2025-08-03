clean:
    rm -rf .cache build

setup:
    CC=clang CXX=clang++ cmake -G Ninja -S . -B build

build:
    cmake --build build

build-release:
    cmake --build build --config Release
