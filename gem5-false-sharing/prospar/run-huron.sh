#!/bin/bash

# HURON Benchmark
# "huron-boost-spinlock" "huron-lockless-micro" "huron-boost-spinlock-manual" "huron-lu-ncb" 
# "huron-boost-test" "huron-lu-ncb-manual" "huron-boost-test-manual" "huron-mrmw"  "huron-false" 
# "huron-mrsw" "huron-false-unrolling" "huron-ref-count" "huron-hist" "huron-string-match" "huron-hist-manual"
# "huron-string-match-manual" "huron-linear-reg" "tmi-boost-refcount" "huron-linear-reg-manual" "huron-locked-micro" "huron-locked-toy" "huron-atomic-lock" "huron-locked-toy-manual"

# tensor:require eigen library to load; atomic lock require sleep noit supported in gem5 
# large-runtime: lockless-micro:16 hr; false-unrolling:16hr; locked-toy:7days; locked-toy-manual:6days; locked-micro:2.5days;
# "huron-locked-toy-manual" "huron-locked-micro" "huron-locked-toy" "huron-atomic-lock"
# add benchmark below to run experiment 
# "huron-boost-spinlock" "huron-boost-spinlock-manual" "huron-boost-test" "huron-boost-test-manual" "huron-lu-ncb" "huron-lu-ncb-manual" "huron-false-unrolling" "huron-lockess-micro"
ALL_BENCHMARKS=()

# SUPPORTED PROTOCOL:"MESI_Two_Level" "MESI_Two_Level_Extended" "FS_MESI_DETECTION_Blocking" "FS_MESI_DETECTION" "FS_MESI_Blocking" "FS_MESI"
ALL_PROTOCOL=()

GRANULARITY=("1") #"2" "4" "8" "16" "32")
INV_THRES=("10000") #"1000" "2000" "4000" "8000" "16000")
FET_THRES=("10000") #"1000" "2000" "4000" "8000" "16000")
ACT_SIZE=("1024") #"1024" "2048")  #("0" "512" "1024" "2048" "4096" "8192")
ACT_ASSOC=("16") #"8" "16") # "4" "32"
SIZE=1024
ASSOC=16

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

for BENCHMARK in "${ALL_BENCHMARKS[@]}"
do
    echo "************ starting ::: $BENCHMARK ************"
    for PROTOCOL in "${ALL_PROTOCOL[@]}"
    do  
        RUN_CONFIG="bash ./huron-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 $GRAIN_SIZE $INV_THRES $FET_THRES $SIZE $ASSOC $SIZE_OWN $RESET_TICK"
        eval "$RUN_CONFIG"
    done
    echo "************ completing ::: $BENCHMARK ************"
done


#For benchmark with diff thread count and input 
# "huron-false" "huron-mrmw" "huron-mrsw" "huron-ref-count" "tmi-boost-refcount" 
#"huron-hist"  "huron-hist-manual" "huron-linear-reg"  "huron-linear-reg-manual" -> input available
# CPU_COUNT=("17" "9" "6" "5" "9" "5" "5")
# "huron-string-match" "huron-string-match-manual" needs file storing key as input,
# skipping now, later fix them
: '
for PROTOCOL in "${ALL_PROTOCOL[@]}"
do
    #huron-false   requires 16 thread while false-unrolling require 4 
    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-false 17"
    #eval "$RUN_CONFIG"
    
    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-mrmw 9"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-mrsw 6"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL tmi-boost-refcount 9"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-ref-count 9"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist 5 histogram small.bmp"
    #eval "$RUN_CONFIG"

    #RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist 5 histogram fs.bmp"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist 5 histogram med.bmp"
    #eval "$RUN_CONFIG"

    #RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist 5 histogram large.bmp"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist-manual 5 histogram small.bmp"
    #eval "$RUN_CONFIG"
    
    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist-manual 5 histogram fs.bmp"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-hist-manual 5 histogram med.bmp"
    #eval "$RUN_CONFIG"

    #RUN_CONFIG="bash ./huron-benchmark-ini.sh $PROTOCOL huron-hist-manual 5 histogram small.bmp"
    #eval "$RUN_CONFIG"
    
    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg 5 linear_regression key_file_50KB.txt"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg 5 linear_regression key_file_50MB.txt"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg 5 linear_regression key_file_100MB.txt"
    #eval "$RUN_CONFIG"
 
    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg 5 linear_regression key_file_500MB.txt"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg-manual 5 linear_regression key_file_50KB.txt"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg-manual 5 linear_regression key_file_50MB.txt"
    #eval "$RUN_CONFIG"

    RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg-manual 5 linear_regression key_file_100MB.txt"
    #eval "$RUN_CONFIG"
 
    #RUN_CONFIG="bash ./input-huron-benchmark.sh $PROTOCOL huron-linear-reg-manual 5 linear_regression key_file_500MB.txt"
    #eval "$RUN_CONFIG"

done
'
