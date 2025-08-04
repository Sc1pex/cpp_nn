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

run_mnist_cli: build_release
    ./build/mnist_cli

download_mnist:
    #!/usr/bin/bash
    set -euo pipefail
    
    rm -rf data
    mkdir -p data
    
    BASE_URL="https://storage.googleapis.com/cvdf-datasets/mnist"
    FILES=(
        "train-images-idx3-ubyte.gz"
        "train-labels-idx1-ubyte.gz"
        "t10k-images-idx3-ubyte.gz"
        "t10k-labels-idx1-ubyte.gz"
    )
    
    # Download each file
    for file in "${FILES[@]}"; do
        curl -L "$BASE_URL/$file" -o "data/$file"
    done

    # Unzip the downloaded files
    for file in "${FILES[@]}"; do
        gunzip "data/$file"
    done
