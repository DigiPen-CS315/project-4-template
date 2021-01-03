#!/usr/bin/env bash

BUILD_CONFIG=${1:-Debug}
BUILD_DIR=${2:-build}

mkdir -p $BUILD_DIR
echo "cmake -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=$BUILD_CONFIG -S . -B$BUILD_DIR"
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_CONFIG -S . -B$BUILD_DIR 

cmake --build $BUILD_DIR --config $BUILD_CONFIG
