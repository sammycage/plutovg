# Build Stage
FROM --platform=linux/amd64 ubuntu:20.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y cmake clang

## Add source code to the build stage.
ADD . /plutovg
WORKDIR /plutovg

## Build
RUN mkdir -p build
WORKDIR build
RUN CC=clang CXX=clang++ cmake ..
RUN make

# Package Stage
FROM --platform=linux/amd64 ubuntu:20.04
COPY --from=builder /plutovg/build/fuzz/plutovg-fuzz /

