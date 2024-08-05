#!/bin/bash

# FEATHERLIGHT Benchmark
# "feather-test1" "feather-test2" "feather-test3" "feather-test4" "feather-test5" "feather-test6" "feather-test7" "feather-test8" "feather-test9" "feather-test10"
# "feather-test1-small" "feather-test2-small" "feather-test3-small" "feather-test4-small" "feather-test5-small" "feather-test6-small" "feather-test7-small" "feather-test8-small" "feather-test9-small"
# copy benchmrk name from above
ALL_BENCHMARKS=()

# SUPPORTED PROTOCOL:
# "MESI_Two_Level" "MESI_Nonblocking" "FS_MESI_DETECTION_Blocking" "FS_MESI_DETECTION" "FS_MESI_Blocking" "FS_MESI"
ALL_PROTOCOL=() # provide name of protocol to run from above

#Metadata Granularity
GRAIN_SIZE=1
# invlaidation counter threshold
INV_THERS=10000
# fetch counter threshold
FET_THRES=10000
# size of global act table
SIZE=1024
# associativity of global act table
ASSOC=16
#size of own access metadata table
SIZE_OWN=512
# number of tick to reset metadata
RESET_TICK=100000

# bash exec_script <protocol_name> <benchmark_name> <num_core> <tracking_width> <inv_thres> <fetch_thres> <act_size> <act_assoc> <own_size> <reset_tick>
for PROTOCOL in "${ALL_PROTOCOL[@]}"
do
    for BENCHMARK in "${ALL_BENCHMARKS[@]}"
    do
        echo "::::::::: starting $BENCHMARK :::::::::"
        RUN_CONFIG="bash ./feather-benchmark-ini.sh $PROTOCOL $BENCHMARK 3 $GRAIN_SIZE $INV_THRES $FET_THRES $SIZE $ASSOC $SIZE_OWN $RESET_TICK"
        eval "$RUN_CONFIG"
        echo "::::::::: stopping $BENCHMARK :::::::::"
    done
done
