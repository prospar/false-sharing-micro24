#!/bin/bash

##########################################################################

# NOTE: Only the following variables need to be updated in most cases
PARTITION="data" # Or data partition

##########################################################################

# https://askubuntu.com/questions/674333/how-to-pass-an-array-as-function-argument
# Array should be the last argument and only one array can be passed
# first parameter is the value to be searched
# second parameter is the array to be searched
function get_array_index() {
  local KEY="$1"
  shift
  local arr=("$@")
  for i in "${!arr[@]}"; do
    if [[ "${arr[$i]}" = "${KEY}" ]]; then
      echo "$i"
    fi
  done
}

# Update the directory path
USER=$(whoami)
INPUT_PROTOCOL=$1
INPUT_BENCH=$2

# SB: I think we cannot have a map data structure with Bash. So, we use two arrays that
# have matching indices.
ALL_BENCHMARKS=("false-sharing" "false-true-sharing-naive" "true-false-sharing" "true-sharing" "without-false-sharing")
BENCH_OPTIONS=("" "" "" "" "")
ALL_PROTOCOLS=("MESI_Two_Level" "MESI_Two_Level_Extended" "FS_MESI_DETECTION" "FS_MESI" "FS_MESI_Blocking")

if [ "${#ALL_BENCHMARKS[@]}" -ne "${#BENCH_OPTIONS[@]}" ]; then
  echo "Size of BENCHMARKS array: ${#ALL_BENCHMARKS[@]}"
  echo "Size of OPTIONS array: ${#BENCH_OPTIONS[@]}"
  exit 1
fi

# https://stackoverflow.com/questions/13470413/converting-a-bash-array-into-a-delimited-string
if [ "$#" -ne 2 ]; then
  echo "Wrong number of parameters to script $0: $#"
  echo "Usage: bash run.sh <protocol> <benchmark>  where "
  echo "<protocol> is one of: $(
    IFS=', '
    echo "[${ALL_PROTOCOLS[*]// / |}]"
    IFS=$''
  )"
  echo "<benchmark> is one of: $(
    IFS=', '
    echo "[${ALL_BENCHMARKS[*]// / |}]"
    IFS=$''
  )"
  exit 1
fi

if [[ ! " ${ALL_PROTOCOLS[*]} " =~ ${INPUT_PROTOCOL} ]]; then
  echo "Wrong protocol name: $(
    IFS=', '
    echo "[${ALL_PROTOCOLS[*]// / |}]"
    IFS=$''
  )"
  exit 1
fi

if [[ ! " ${ALL_BENCHMARKS[*]} " =~ ${INPUT_BENCH} ]]; then
  echo "Wrong benchmark name: $(sc  
    IFS=', '
    echo "[${ALL_BENCHMARKS[*]// / |}]"
    IFS=$''
  )"
  exit 1
fi

BENCHMARKS_DIR="/$PARTITION/$USER/false-sharing-benchmarks"
GEM5_DIR="/$PARTITION/$USER/GEM5-FS/gem5-false-sharing"

# Build benchmarks

cd "$BENCHMARKS_DIR" || exit
mkdir -p build
cd build || exit
cmake ..
cmake --build .

BENCHMARKS_BIN_DIR="$BENCHMARKS_DIR/build/bin"

# Build gem5

cd "$GEM5_DIR" || exit

BUILD_CMD="scons build/X86_$INPUT_PROTOCOL/gem5.opt -j4 --default=X86 PROTOCOL=$INPUT_PROTOCOL SLICC_HTML=True"
eval "$BUILD_CMD"

# Create output directory structure

NOW=$(date +'%F-%T' | tr : -)
OUTDIR="output/$INPUT_BENCH/$INPUT_PROTOCOL/$NOW"

mkdir -p OUTDIR

# 8 core 2 GHz
# 32 KB 8-way associative L1I
# 32 KB 8-way associative L1D
# 8 MB 16-way LLC (L2)
# 16 GB memory

NUM_CPUS=8
CACHELINE_SIZE=64
CPU_TYPE="DerivO3CPU"

PRIMARY_MEM=16GB

L1D_ASSOC=8
L1D_SIZE=32kB

L1I_ASSOC=8
L1I_SIZE=32kB

L2_ASSOC=16
L2_SIZE=8MB

# TODO: Set up a map from benchmarks to options

INDEX=$(get_array_index "$INPUT_BENCH" "${ALL_BENCHMARKS[@]}")
BENCH_OPTION="${BENCH_OPTIONS[$INDEX]}"

# Run benchmark

CMD="build/X86_$INPUT_PROTOCOL/gem5.opt --outdir=$OUTDIR configs/example/se.py --ruby --cpu-type='$CPU_TYPE' --mem-size='$PRIMARY_MEM' --num-cpus=$NUM_CPUS --cacheline_size=$CACHELINE_SIZE --l1d_assoc=$L1D_ASSOC --l1d_size=$L1D_SIZE --l1i_assoc=$L1I_ASSOC --l1i_size=$L1I_SIZE --l2_assoc=$L2_ASSOC --l2_size=$L2_SIZE --cmd='$BENCHMARKS_BIN_DIR/$INPUT_BENCH' --options='$BENCH_OPTION'"

eval "$CMD"
