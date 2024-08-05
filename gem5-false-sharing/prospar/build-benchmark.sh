#!/bin/bash
# one time activity for each 
eval "$(cat ./fs.ini | python3 ./cfgparser.py)"

BENCHMARKS_DIR="${PROJECT[benchmarks_root]}"

cd "$BENCHMARKS_DIR" || exit
mkdir -p build
cd build || exit
cmake ..
cmake --build .

BENCHMARKS_BIN_DIR="$BENCHMARKS_DIR/build/bin"