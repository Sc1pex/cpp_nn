clean:
    rm -rf .cache build

setup:
    CC=clang CXX=clang++ cmake -G Ninja -S . -B build

build:
    cmake --build build

build_release:
    cmake --build build --config Release

run_xor: build_release
    ./build/nnet/xor_test

run_mnist_benchmark: build_release
    ./build/nnet/mnist_benchmark

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

becnhmark_mnist_train: build_release
    #!/bin/bash

    output_file=$(mktemp)

    hyperfine \
        --shell bash \
        "./build/nnet/mnist_benchmark >> $output_file" \
        --runs 10

    # Extract the numbers from all runs
    numbers=$(grep -o '[0-9]\+\(\.[0-9]\+\)\?' "$output_file")
    # Calculate statistics using awk
    value_stats=$(echo "$numbers" | awk '
    {
        sum += $1
        values[NR] = $1
        count = NR
    }
    END {
        if (count > 0) {
            mean = sum / count
            # Calculate variance for standard deviation
            for (i = 1; i <= count; i++) {
                variance += (values[i] - mean) ^ 2
            }
            stddev = sqrt(variance / count)
            printf "Mean value: %.6f ± %.6f", mean, stddev
        }
    }')

    # Display value statistics
    echo -e "\n=== Output Value Statistics ==="
    echo "$value_stats"

    # Cleanup
    rm "$output_file"
