#!/bin/bash

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

# Parse INI file

# https://serverfault.com/questions/345665/how-to-parse-and-convert-ini-file-into-bash-array-variables
eval "$(cat ./fs.ini | python3 ./cfgparser.py)"

# Update the directory path
USER=$(whoami)
INPUT_PROTOCOL=$1
INPUT_BENCH=$2

# SB: I think we cannot have a map data structure with Bash. So, we use two arrays that
# have matching indices.
ALL_BENCHMARKS=("false-sharing" "fs-ts-sameline" "fs-ts-diffline" "true-sharing" "no-false-sharing" "both-false-and-true-sharing" "both-true-and-false-sharing" "proportional-fs" "proportional-ts"\
 "repetitive-fs-ts-sameline" "array-lock" "repetitive-fs-ts-diffline")

# change benchoption to control number of iteration for each protocol
BENCH_OPTIONS=("18" "18" "18" "18" "18" "18" "18" "18" "18" "18" "18" "18")
ALL_PROTOCOLS=("MESI_Two_Level" "FS_MESI_DETECTION_Blocking" "FS_MESI_DETECTION" "FS_MESI" "FS_MESI_Blocking" "MESI_Nonblocking" "DETECT_MD")

if [ "${#ALL_BENCHMARKS[@]}" -ne "${#BENCH_OPTIONS[@]}" ]; then
  echo "Size of BENCHMARKS array: ${#ALL_BENCHMARKS[@]}"
  echo "Size of OPTIONS array: ${#BENCH_OPTIONS[@]}"
  exit 1
fi

# https://stackoverflow.com/questions/13470413/converting-a-bash-array-into-a-delimited-string
# vipin: three param now number of cores also
#if [ "$#" -ne 2 ]; then
#  echo "Wrong number of parameters to script $0: $#"
#  echo "Usage: bash run.sh <protocol> <benchmark>  where "
#  echo "<protocol> is one of: $(
#    IFS=', '
#    echo "[${ALL_PROTOCOLS[*]// / |}]"
#    IFS=$''
#  )"
#  echo "<benchmark> is one of: $(
#    IFS=', '
#    echo "[${ALL_BENCHMARKS[*]// / |}]"
#    IFS=$''
#  )"
#  exit 1
#fi

if [[ ! " ${ALL_PROTOCOLS[*]} " =~ ${INPUT_PROTOCOL} ]]; then
  echo "Wrong protocol name: $(
    IFS=', '
    echo "[${ALL_PROTOCOLS[*]// / |}]"
    IFS=$''
  )"
  exit 1
fi

if [[ ! " ${ALL_BENCHMARKS[*]} " =~ ${INPUT_BENCH} ]]; then
  echo "Wrong benchmark name: $(
    IFS=', '
    echo "[${ALL_BENCHMARKS[*]// / |}]"
    IFS=$''
  )"
  exit 1
fi

BENCHMARKS_DIR="${PROJECT[benchmarks_root]}"
GEM5_DIR="${PROJECT[fs_gem5_root]}"

# Activate virtual environment

source "${PROJECT[virtualenv_root]}/bin/activate"

# Build benchmarks once at start

BENCHMARKS_BIN_DIR="$BENCHMARKS_DIR/build/bin"

# Build gem5

cd "$GEM5_DIR" || exit

BUILD_CMD="scons build/X86_$INPUT_PROTOCOL/gem5.opt -j$3 --default=X86 PROTOCOL=$INPUT_PROTOCOL SLICC_HTML=True"
eval "$BUILD_CMD"
# uncomment the above command later
# Create output directory structure

NOW=$(date +'%F-%T' | tr : -)
OUTDIR="${PROJECT[exp_output_root]}/$INPUT_BENCH/$INPUT_PROTOCOL/$NOW"

mkdir -p "$OUTDIR"

# We should choose a memory subsystem configuration that can avoid pressure on the memory
# subsystem. For a n-thread simulation, we should use a n-core configuration. We should use a
# 1-to-1 mapping between cores and threads, and it is okay to use 2MB caches per core.

# 8 core 2 GHz
# 32 KB 8-way associative L1I
# 32 KB 8-way associative L1D
# 16 MB 16-way LLC (L2)
# 32 GB memory


NUM_CPUS=$3

CACHELINE_SIZE=64
CPU_TYPE="DerivO3CPU"

PRIMARY_MEM=32GB

L1D_ASSOC=8
L1D_SIZE=32kB

L1I_ASSOC=8
L1I_SIZE=32kB

L2_ASSOC=16
L2_SIZE=16MB

GRANULARITY=$4
INV_TH=$5
FETCH_TH=$6
ACT_SIZE=$7
ASSOC_ACT=$8
SIZE_OWN=$9
RESET_TICK=${10}

INDEX=$(get_array_index "$INPUT_BENCH" "${ALL_BENCHMARKS[@]}")
BENCH_OPTION="${BENCH_OPTIONS[$INDEX]}"

# Run benchmark

# --debug-flags=ProtocolTrace --debug-file=deadlockLog
# --debug-flags=ProtocolTrace --debug-file=Log
# --debug-flag=PrvDebug check once for eviction of block with pending MD count
CMD="build/X86_$INPUT_PROTOCOL/gem5.opt --outdir=$OUTDIR configs/example/se.py \
 --ruby --cpu-type='$CPU_TYPE' --mem-size='$PRIMARY_MEM' --num-cpus=$NUM_CPUS \
 --cacheline_size=$CACHELINE_SIZE --l1d_assoc=$L1D_ASSOC --l1d_size=$L1D_SIZE \
 --l1i_assoc=$L1I_ASSOC --l1i_size=$L1I_SIZE --l2_assoc=$L2_ASSOC \
 --l2_size=$L2_SIZE --tracking_width=$GRANULARITY --inv_threshold=$INV_TH \
 --fetch_threshold=$FETCH_TH --global_act_size=$ACT_SIZE --assoc_act=$ASSOC_ACT --size_own=$SIZE_OWN --reset_tick=$RESET_TICK --cmd='$BENCHMARKS_BIN_DIR/$INPUT_BENCH' \
 --options='$BENCH_OPTION'"

 eval "$CMD"
