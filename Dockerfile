FROM alpine:3.21 AS cpp-builder

RUN apk add --no-cache \
    cmake \
    clang \
    clang-extra-tools \
    git \
    ninja \
    build-base \
    linux-headers

WORKDIR /build

COPY CMakeLists.txt CMakePresets.json ./
COPY nn_lib/ ./nn_lib/
COPY nn_web/server/ ./nn_web/server/

RUN cmake --preset release \
    && cmake --build --preset release

FROM alpine:3.21 AS data-downloader

RUN apk add --no-cache \
    curl

WORKDIR /data

RUN mkdir -p mnist_data && \
    cd mnist_data && \
    curl -L "https://storage.googleapis.com/cvdf-datasets/mnist/train-images-idx3-ubyte.gz" -o "train-images-idx3-ubyte.gz" && \
    curl -L "https://storage.googleapis.com/cvdf-datasets/mnist/train-labels-idx1-ubyte.gz" -o "train-labels-idx1-ubyte.gz" && \
    curl -L "https://storage.googleapis.com/cvdf-datasets/mnist/t10k-images-idx3-ubyte.gz" -o "t10k-images-idx3-ubyte.gz" && \
    curl -L "https://storage.googleapis.com/cvdf-datasets/mnist/t10k-labels-idx1-ubyte.gz" -o "t10k-labels-idx1-ubyte.gz" && \
    gunzip *.gz

FROM oven/bun:1-alpine AS ui-builder

WORKDIR /build

COPY nn_web/ui/package.json nn_web/ui/bun.lockb* ./
COPY nn_web/ui/ ./

RUN bun install
RUN bun run build

FROM alpine:3.21

RUN apk add --no-cache \
    libstdc++

WORKDIR /app

COPY --from=cpp-builder /build/build/release/nn_web/server/nn_server ./
COPY --from=ui-builder /build/dist ./assets
COPY --from=data-downloader /data/mnist_data ./mnist_data

ENV ASSETS_PATH=/app/assets

EXPOSE 8080

CMD ["./nn_server"]
