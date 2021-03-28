#!/bin/bash

build_type=$1
num_thread=$2

result_folder=results/"$build_type"-"$num_thread"
mkdir -p "$result_folder"

mkdir -p build
rm -rf build/*
cd build || exit
cmake .. -DCMAKE_BUILD_TYPE="$build_type" -DCMAKE_EXPORT_COMPILE_COMMANDS=True
make -j"$num_thread"
cd ..
cp build/compile_commands.json "$result_folder"
cp build/src/conceal* "$result_folder"
cp build/src/walletd "$result_folder"
