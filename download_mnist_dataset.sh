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

for file in "${FILES[@]}"; do
    curl -L "$BASE_URL/$file" -o "data/$file"
done

for file in "${FILES[@]}"; do
    gunzip "data/$file"
done
