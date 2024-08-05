#!/bin/bash

# Microbenchmark 
#"false-sharing-with-eviction"
# "false-sharing" "false-true-sharing-naive" "true-false-sharing" "true-sharing" "without-false-sharing" "false-sharing"
# copy benchmark name from above and run
ALL_BENCHMARKS=()

# SUPPORTED PROTOCOL:
# "MESI_Two_Level" "MESI_Nonblocking" "FS_MESI_DETECTION_Blocking" "FS_MESI_DETECTION" "FS_MESI_Blocking" "FS_MESI"
ALL_PROTOCOL=() 

GRANULARITY=("1") # "2" "4" "8" "16" "32")
INV_THRES=("10000") #"1000" "2000" "4000" "8000" "16000")
FETCH_THRES=("10000") #"1000" "2000" "4000" "8000" "16000")
ACT_SIZE=("1024") # "1024" "2048")  #("0" "512" "1024" "2048" "4096" "8192")
ACT_ASSOC=("16") #"8" "16") # "4" "32"

for BENCHMARK in "${ALL_BENCHMARKS[@]}"
do
	echo "*******************BENCHMARK $BENCHMARK started*****************************"
	for PROTOCOL in "${ALL_PROTOCOL[@]}"
	do
		for SIZE in "${ACT_SIZE[@]}"
		do
			for ASSOC in "${ACT_ASSOC[@]}"
			do
				#echo "================START ========================"
				#echo "================ GRAN 1========================"
				RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 1 10000 10000 $SIZE $ASSOC 512 100000"
				eval "$RUN_CONFIG"
				#echo "================GRAN 2========================"
				RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 2 10000 10000 $SIZE $ASSOC 512 100000"
				#eval "$RUN_CONFIG"
				#echo "================GRAN 4========================"
				RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 4 10000 10000 $SIZE $ASSOC 512 100000"
				#eval "$RUN_CONFIG"
				#echo "================GRAN 8========================"
				RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 8 10000 10000 $SIZE $ASSOC 512 100000"
				#eval "$RUN_CONFIG"
				#echo "================GRAN 16========================"
				RUN_CONFIG="bash ./run-benchmark-ini.sh $PROTOCOL $BENCHMARK 5 16 10000 10000 $SIZE $ASSOC 512 100000"
				#eval "$RUN_CONFIG"
				#echo "================GRAN 32========================"
				#echo "================== END ============================="
			done
		done
		echo "*******************$PROTOCOL COMPLETED*****************************"
    	done
    echo "************* $BENCHMARK COMPLETED******************************"
done

# argument_position | factor
#       1			| protocol_name
#       2			| benchmark_name
#		3			| num_cpus
#		4			| grainularity (metadata width optimization)
#       5			| invalidation threshold
#       6			| fetch threshold
#		7			| size of global AcT table
#		8			| associativity of global AcT table
