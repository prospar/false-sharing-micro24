# False Sharing Benchmarks

## Folder Description

- `microbenchmarks`: contains small programs with known behavior to test the correctness of our approach
- `benchmarks`: contains applications from benchmark suites like PHOENIX, Synchrobench listed by prior work

## Build

#### For FS Mode:
<!--`mkdir -p build; cd build; rm -rf *; cmake -DGEM5_PATH:PATH="path_to_gem5" ..; cmake --build .`-->

- `CUSTOM_HOOK` set the m5_library path and enable `ROI_TRACING` flag <br>

`mkdir -p build; cd build; rm -rf *; cmake -DCUSTOM_HOOK=true ..; cmake --build .`

- set `CUSTOM_HOOK` to false if not intending to use ROI specific stats.

- To make static binary for FS mode:
    `mkdir -p build; cd build; rm -rf *; cmake -DSTATIC_BINARY=true ..; cmake --build .`


#### For SE Mode:
    `mkdir -p build; cd build; rm -rf *; cmake -DSE_MODE=true ..; cmake --build .`

## Microbenchmarks

- List of programs

  - `false_sharing.c`: `gcc -Wall -O0 -pthread false_sharing.c`
  - `true_sharing.c`: `gcc -O0 -Wall -pthread true_sharing.c`
  - `false_true_sharing_naive.c`: `gcc -O0 -Wall -pthread false_true_sharing_naive.c`
  - `true_false_sharing.c`: `gcc -O0 -Wall -pthread true_false_sharing.c`

- Compile each microbenchmark using `gcc` without any optimization
- GCC with optimizations enabled removes padding. This might affect program behavior.

## Benchmarks

- Application suite provided by existing research work
  - `huron`
    - Program provided at <https://github.com/efeslab/huron/tree/master/test_suites> + microbenchmarks shared by Tanveer Ahmed Khan
    - Did not include `histogram` and `linear_regression` inputs due to large size
  - [`synchrobench`](https://github.com/gramoli/synchrobench) is a data structure program, refer to [`featherlight`](https://github.com/WitchTools/Feather) paper for more details

### Instruction to build hooks to track ROI

- `gcc -fPIC -I.. -c -o hooks_prospar.o hooks_prospar.c`
- `gcc -fPIC -I./ -c -DM5OP_ADDR=0xFFFF0000 -DM5OP_PIC -O2 -o m5op_x86.o m5op_x86.S`
- `gcc -fPIC -I.. -c -o m5_mmap.o m5_mmap.c`
- `ar rcs libhooks_prospar.a hooks_prospar.o m5op_x86.o m5_mmap.o`

- `nm libhooks_prospar.a`

- `gcc -c -DROI_TRACING -I<path_to_m5_library> false-sharing.c -o false-sharing.o`
- `gcc -o false-sharing false-sharing.o -L/usr/lib/x86_64-linux-gnu -L<path_to_m5_library_folder> -l:libhooks_prospar.a -pthread`

### Instruction to build synchrobench with hooks to track ROI

- Build `libhooks_prospar` library using the cmake.
- Update the `Makefile.common` file in `synchrobench/c-cpp/common/`
  ```make
  ifdef CUSTOM_HOOK
  M5_PATH?=/home/prospar/false-sharing-benchmarks
  export M5_PATH
  CFLAGS += -I${M5_PATH}/m5_library -DROI_TRACING
  LDFLAGS += -L${M5_PATH}/build/m5_library -llibhooks_prospar
  endif
  ```
- To build run `make CUSTOM_HOOK=true M5_PATH=<path_to_m5_library>`


#### Instruction to build hooks to track ROI:
- gcc -fPIC -I.. -c -o hooks_prospar.o hooks_prospar.c
- gcc -fPIC -I./ -c -DM5OP_ADDR=0xFFFF0000 -DM5OP_PIC -O2 -o m5op_x86.o m5op_x86.S
- gcc -fPIC -I.. -c -o m5_mmap.o m5_mmap.c
- ar rcs libhooks_prospar.a hooks_prospar.o m5op_x86.o m5_mmap.o

