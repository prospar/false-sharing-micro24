#!/bin/bash

# Benchmark binaries

# eval "$(cat ./fs.ini | python3 ./cfgparser.py)"

# BENCHMARK_DIR="${PROJECT[benchmarks_root]}"

# cd "$BENCHMARK_DIR" || exit
# mkdir -p build
# cd build || exit
# cmake ..
# cmake --build .

# Run all benchmark and protocol

ALL_BENCHMARKS=("feather-test1" "feather-test2" "feather-test3" "feather-test4" "feather-test5" "feather-test6" "feather-test7" "feather-test8" "feather-test9")

#Modify or leave out the protocol you dont want to execute
# "MESI_Two_Level" "MESI_Two_Level_Extended" "FS_MESI_DETECTION" "FS_MESI_Blocking" "FS_MESI")
ALL_PROTOCOL=("FS_MESI" "MESI_Two_Level")

for PROTOCOL in "${ALL_PROTOCOL[@]}"
do
	for BENCHMARK in "${ALL_BENCHMARKS[@]}"
	do
		#echo "$PROTOCOL $BENCHMARK"
		RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK"
		eval "$RUN_CONFIG"
		# eval "$(bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK)"
	done

done

# bash ./run-benchmark-ini.sh MESI_Two_Level false-sharing
# bash ./run-benchmark-ini.sh MESI_Two_Level_Extended false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_DETECTION false-sharing
# bash ./run-benchmark-ini.sh FS_MESI false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_Blocking false-sharing

# bash ./run-benchmark-ini.sh MESI_Two_Level false-true-sharing-naive
# bash ./run-benchmark-ini.sh MESI_Two_Level_Extended false-true-sharing-naive
# bash ./run-benchmark-ini.sh FS_MESI_DETECTION false-true-sharing-naive
# bash ./run-benchmark-ini.sh FS_MESI false-true-sharing-naive 
# bash ./run-benchmark-ini.sh FS_MESI_Blocking false-true-sharing-naive


# bash ./run-benchmark-ini.sh MESI_Two_Level true-false-sharing
# bash ./run-benchmark-ini.sh MESI_Two_Level_Extended true-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_DETECTION true-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI true-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_Blocking true-false-sharing

# bash ./run-benchmark-ini.sh MESI_Two_Level true-sharing
# bash ./run-benchmark-ini.sh MESI_Two_Level_Extended true-sharing
# bash ./run-benchmark-ini.sh FS_MESI_DETECTION true-sharing
# bash ./run-benchmark-ini.sh FS_MESI true-sharing
# bash ./run-benchmark-ini.sh FS_MESI_Blocking true-sharing

# bash ./run-benchmark-ini.sh MESI_Two_Level without-false-sharing
# bash ./run-benchmark-ini.sh MESI_Two_Level_Extended without-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_DETECTION without-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI without-false-sharing
# bash ./run-benchmark-ini.sh FS_MESI_Blocking without-false-sharing

