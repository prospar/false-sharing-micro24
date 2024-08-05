# PROSPAR False Sharing README

## Clone gem5

Public repository: `git clone https://gem5.googlesource.com/public/gem5`

False Sharing repository: `git clone git@git.cse.iitk.ac.in:prospar/gem5-false-sharing.git`

## BUILD

Install `gem5` dependencies.

Ubuntu 18.04 and Python 2 virtualenv

`sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python-dev python python-six python libboost-all-dev pkg-config automake python-protobuf python-pydot python-pydot-ng graphviz libhdf5-dev`

Ubuntu 18.04 and Python3 virtualenv

`sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python3-dev python3 python3-six python libboost-all-dev pkg-config automake python3-protobuf python-pydot python-pydot-ng graphviz libhdf5-dev`

Ubuntu-20.04

`sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python3-dev python3 python3-six python-is-python3 libboost-all-dev pkg-config automake python3-protobuf python-pydot python-pydot-ng graphviz libhdf5-dev`

Create a `Python 2` virtual environment for `gem5` versions < 21 and a `Python 3` environment for newer versions.

```bash
mkdir $HOME/virtualenvs && cd virtualenvs
python3 -m pip install -U virtualenv
virtualenv -p /usr/bin/python2 fs-python2-venv
source fs-python2-venv/bin/activate
pip install -U pip setuptools six scons pydot
```

You may encounter the following error while trying to create a Python 2.7 virtual environment on Ubuntu 22+.

```shell
RuntimeError: failed to query /usr/bin/python2.7 with code 1 err: '  File "/usr/local/lib/python3.10/dist-packages/virtualenv/discovery/py_info.py", line 152\n    os.path.join(base_dir, exe) for exe in (f"python{major}", f"python{major}.{minor}")\n                                                           ^\nSyntaxError: invalid syntax\n'
```

The reason is `virtualenv` versions >= 20.22.0 dropped support for creating Python environments for Python versions <= 3.6. So you will need to downgrade `virtualenv`: `python3 -m pip install virtualenv==20.21.1`.

```bash
cd gem5; scons build/X86/gem5.opt -j<num_core>
```

```bash
cd gem5; scons build/X86_<protocol_name>/gem5.opt -j<num_core> PROTOCOL=<protocol_name> SLICC_HTML=<if True, Generate HTML files for protocol> --default=X86
```

You will have to compile `gem5` separately for every ISA that you want to simulate. If using Ruby, you have to have separate compilations for every cache coherence protocol.

Example

```shell
scons build/X86/gem5.opt -j4 --default=X86
```

```shell
bear -- scons build/X86/gem5.opt -j4 --default=X86
```

Blocking MESI baseline

```shell
scons build/X86_MESI_Two_Level/gem5.opt -j4 --default=X86 PROTOCOL=MESI_Two_Level SLICC_HTML=True
```

```shell
bear -- scons build/X86_MESI_Two_Level/gem5.opt -j4 --default=X86 PROTOCOL=MESI_Two_Level SLICC_HTML=True
```

Nonblocking MESI extension

```shell
scons build/X86_MESI_Two_Level_Extended/gem5.opt -j4 --default=X86 PROTOCOL=MESI_Two_Level_Extended SLICC_HTML=True
```

```shell
bear -- scons build/X86_MESI_Two_Level_Extended/gem5.opt -j4 --default=X86 PROTOCOL=MESI_Two_Level_Extended SLICC_HTML=True
```

False Sharing detection

```shell
scons build/X86_FS_MESI_DETECTION/gem5.opt -j4 --default=X86 PROTOCOL=FS_MESI_DETECTION SLICC_HTML=True
```

```shell
bear -- scons build/X86_FS_MESI_DETECTION/gem5.opt -j4 --default=X86 PROTOCOL=FS_MESI_DETECTION SLICC_HTML=True
```

False Sharing w/ privatization

```shell
scons build/X86_FS_MESI/gem5.opt -j4 --default=X86 PROTOCOL=FS_MESI SLICC_HTML=True
```

```shell
bear -- scons build/X86_FS_MESI/gem5.opt -j4 --default=X86 PROTOCOL=FS_MESI SLICC_HTML=True
```

Active, supported ISAs include: ARM, RISC-V, SPARC, X86. `gem5` can also run ALPHA, MIPS, and POWER ISAs but they are no longer maintained and thus should be used with caution and extreme testing.

- Running benchmarks
  - `build/X86/gem5.opt configs/learning_gem5/part1/simple.py`
  - `build/X86/gem5.opt configs/learning_gem5/part1/two_level.py --l2_size='1MB' --l1d_size='128kB'`
  - `build/X86/gem5.opt configs/example/se.py --cmd=tests/test-progs/hello/bin/x86/linux/hello`
  - `build/X86/gem5.opt configs/example/se.py --cmd=tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --l1d_size=64kB --l1i_size=16kB --caches`
  - `build/X86/gem5.opt configs/example/se.py --help`
  - `build/X86/gem5.opt --debug-flags=DRAM configs/learning_gem5/part1/simple.py`
  - `build/X86/gem5.opt --debug-help`

Other choices for `debug-flags` are `Exec` which shows details of how each instruction is executed by the simulated CPU.

Binary types refer to the level of debugging and optimization information in the compiled binary. `gem5` supports 5 binary types.

- `debug` - no optimizations and all debug symbols. The resulting binary is very slow but enables use of the debugger. Good for when you want to debug new changes you’ve made but not ideal for running longer simulations.
- `opt` - most optimizations and all debug symbols. Results in a much faster binary than the debug type but also allows for a degree of debugging and issue tracking.
- `fast` - all optimizations and no debug symbols. This is the fastest possible binary and has the smallest size but provides no debug information if issues occur. Good for running unmodified, vanilla gem5 runs where no issues are likely to occur or when you have the utmost faith in the changes you’ve made to the source.
- `prof` - contains profiling information to be used with `gprof` (GNU profiler)
- `perf` - contains profiling information to be used with `gperftools` (Google performance tools)

The three CPU models `TimingSimpleCPU`, `O3CPU`, and `AtomicSimpleCPU`. By default, gem5 uses the atomic CPU and uses atomic memory accesses.

## Detecting False Sharing

Build only the false sharing detection protocol: `bear -a -vv scons build/X86/gem5.opt -j4 --default=X86 PROTOCOL=FS_MESI_DETECTION SLICC_HTML=True`.

## Modifications related to False Sharing

Search with the string `// FalseSharing:` to check modified code.

## Executing Applications

`build/X86_protocol_name/gem5.opt <gem5_cmd_options> configs/example/se.py --cmd=<path_to_binary> --options=<cmd_arg_for binary> <system_config_options>`

`<gem5_binary> <options> <system_script> --options="input_file iteration_count"`

`build/X86_protocol_name/gem5.opt <options_for_gem5> <system_script> <option_for_script>`

### Microbenchmark

`build/X86_MESI_Two_Level/gem5.opt configs/example/se.py --ruby --cpu-type="DerivO3CPU" --mem-size="8GB" --num-cpus=5 --cacheline_size=64 --cmd="/data/swarnendu/prospar-workspace/false-sharing-benchmarks/build/bin/true-false-sharing"`

### Histogram

`build/X86_MESI_Two_Level/gem5.opt --outdir="test_run/<output_folder>" --debug-flags=ProtocolTrace --debug-file=DebugLog --debug-start=<tick_to_start_logging> configs/example/se.py --ruby --cpu-type="DerivO3CPU" --mem-size="8GB" --num-cpus=5 --cacheline_size=64 --cmd="/data/vipinpat/program_bm/test_suites/histogram/hist" --options="/data/vipinpat/program_bm/test_suites/histogram/input/small.bmp 1" --l1i_assoc=8 --l1d_assoc=8 --l1i_size=32kB --l1d_size=32kB --l2_size=2MB --l2_assoc=16`

To disable debugging, remove all debugging flags from `--debug-flag option`

`./histogram <input_file> <iteration_count>`

```bash
build/X86_FS_MESI/gem5.opt --outdir="fs_hist_out" --debug-flags=GlobalAccess,OverlapDetection --debug-file=ODDebugOut configs/false_sharing_project/se.py --cpu-type="DerivO3CPU" --mem-size="4GB" --num_cpus=4 --num-cpus=4 --cacheline_size=64 --cmd="path_to_binary" --options="cmd_param_for_binary" --l1i_assoc=8 --l1d_assoc=8
```

### Linear Regression

## gem5 Tutorial

- TutorialLink:<https://generic.wordpress.soton.ac.uk/arm/wp-content/uploads/sites/360/2016/10/gem5_tutorial.pdf>
- `build/X86/gem5.opt configs/learning_gem5/part1/simple.py` `build/X86/gem5.opt configs/learning_gem5/part1/two_level.py`
- `build/X86/gem5.opt configs/example/se.py --cmd=tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --l1d_size=64kB --l1i_size=16kB --caches`

`build/X86_FS_MESI/gem5.opt --outdir="fs_hist_out" --debug-flags=GlobalAccess,OverlapDetection --debug-file=ODDebugOut configs/false_sharing_project/fs.py --kernel=<path_to_kernel> --disk-image="<path_to_image>" --cpu-type="DerivO3CPU" --mem-size="4GB" --num_cpus=4 --num-cpus=4 --cacheline_size=64 --cmd= "path_to_binary" --options="cmd_param_for_binary" --l1i_assoc=8 --l1d_assoc=8`

### Multicore Full System Simulation with gem5

- <https://www.gem5.org/project/2020/03/09/boot-tests.html>
- <https://www.gem5.org/documentation/general_docs/checkpoints/>

## Understanding gem5 source code

- Important files
  - `src/mem/ruby/protocol/FS_MESI-msg.sm`
  - `src/mem/ruby/protocol/FS_MESI-dir.sm`
  - `build/X86/mem/ruby/protocol/Directory_State.cc`
  - `build/X86/mem/ruby/protocol/AccessPermission.hh`
  - `build/X86/mem/ruby/protocol/Directory_Controller.cc` `build/X86/gem5.opt configs/learning_gem5/part1/simple.py` `build/X86/gem5.opt configs/learning_gem5/part1/two_level.py` `build/X86/gem5.opt configs/example/se.py --cmd=tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --l1d_size=64kB --l1i_size=16kB --caches`

## Troubleshooting the Build

Make sure you do not have a very recent `protobuf` installation. Version `3.6.x` works but version `3.12.x` led to linking errors.

Delete the relevant directory under `./build` if you get errors due to old files generated with an older `protoc` compiler.

### Issues in system setup for FS mode

- Kernel panic while booting: File system version mismatch. setup the file system first then boot the kernel
- KeyError : Check for keyValue pair returned for different configuration. For Ex: KeyError 'S': at "/home/vipin/py2VirEnv/Gem5-FS/new_repo/gem5-false-sharing/SConstruct" was due to a transition "transition(PR_MTE, {WB_Data, WB_Data_clean}, S)" `>> L2 cache has state SS instead of S Correct transition "transition(PR_MTE, {WB_Data, WB_Data_clean}, SS)"`

## Debugging gem5 with gdb

1. Attach gdb to gem5 binary: `gdb build/X86_<PROTOCOL_NAME>/gem5.opt`
2. Once gdb completes reading the binary, set run time argument. For example, `--outdir=<PATH_TO_OUTPUT_DIR> configs/example/se.py --ruby --cpu-type=DerivO3CPU --mem-size=16GB --num-cpus=5 --cacheline_size=64 --l1d_assoc=8 --l1d_size=32kB --l1i_assoc=8 --l1i_size=32kB --l2_assoc=16 --l2_size=16MB --cmd=<PATH_TO_BINARY>`
3. Set up a breakpoint; the breakpoint is set in the corresponding C/C++ file in the `build/X86_PROTOCOL_NAME` directory.

   `(gdb) break L1Cache_Controller::d_sendDataToRequestor`

4. Set the condition for the breakpoint: As there can be multiple forwarded requests being handled in the system, we need to filter out the block address participating in the deadlock.

```bash
(gdb) info break -> list down all the break point setup for debugging
(gdb) condition <break_point_num> addr == <numerical value of block address (in base 10)>
```

5. On my lab-system the deadlock is reported for block 0x2200. So I will set the breakpoint condition as `condition <breakpoint_number> addr == 8704`
6. Continue debugging with the `next` or `step` command

## Machine SETUP

- Create a virtual environment with python (for ex: python2 exe in /usr/bin/python2 and virtual env name: gem5-venv)

```
virtualenv -p /usr/bin/python2 gem5-venv
```

- Activate virtual environment

```
~$ cd gem5-venv
~$ source bin/activate
```

- Install gem5-dependency Ubuntu-18.04

```
(python2)
(gem5-venv) ~$ sudo apt install build-essential git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python-dev python-six python libboost-all-dev pkg-config
(python3)
(gem5-venv) ~$ sudo apt install build-essential git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python3-dev python3-six python libboost-all-dev pkg-config
```

Ubuntu-20.04

```
(gem5-venv) ~$ sudo apt install build-essential git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python3-dev python3-six python-is-python3 libboost-all-dev pkg-config
```

- Clone the false sharing repo

```
(gem5-venv) ~$ git clone git@git.cse.iitk.ac.in:prospar/gem5-false-sharing.git
```

- Build a gem5 binary

```
(gem5-venv) ~$ scons build/X86_<protocol_name>/gem5.opt -j<num_core> PROTOCOL=<protocol_name> --default=X86
```

- Executing a benchmark

```shell
(gem5-venv) ~$ build/X86_protocol_name/gem5.opt <gem5_cmd_options> configs/example/se.py --cmd=<path_to_binary>\
 --options=<cmd_arg_for binary> <system_config_options>

Example:
build/X86_MESI_Two_Level/gem5.opt --outdir="test_run/<output_folder>" --debug-flags=ProtocolTrace --debug-file=DebugLog --debug-start=<tick_to_start_logging> configs/example/se.py --ruby --cpu-type="DerivO3CPU" --mem-size="8GB" --num-cpus=5 --cacheline_size=64 --cmd="/data/vipinpat/program_bm/test_suites/histogram/hist" --options="/data/vipinpat/program_bm/test_suites/histogram/input/small.bmp 1" --l1i_assoc=8 --l1d_assoc=8 --l1i_size=32kB --l1d_size=32kB --l2_size=2MB --l2_assoc=16
```

To disable debugging: remove all debugging flags from `--debug-flag`.vscode/ option

## Debugging Steps

The different debug flags can be found in `src/mem/ruby/Sconscript` file.

- Generate application binary
  - Static: `gcc -static -o <exe_name> filename -pthread`
  - Dynamic: `gcc -o <exe_name> filename -pthread`

Execution command: `build/X86_MESI_Two_Level_Extended/gem5.opt --debug-flag=<Set of debug flag> --debug-file=<Log_file> --outdir=<output_dir> configs/example/se.py --ruby --cpu-type=DerivO3CPU --mem-size=16GB --num-cpus=5 --cacheline_size=64 --l1d_assoc=8 --l1d_size=32kB --l1i_assoc=8 --l1i_size=32kB --l2_assoc=16 --l2_size=16MB --cmd=<path_to_binary> --options=""`

For enabling all ruby debugging flags, we can set `-debug-flag=Ruby`. It will enable all debugging flag corresponding to Ruby structures, but the size of the generated debug log will be huge. The `ProtocolTrace` flag needs to be specified explicitly for state transition. Multiple flags are provided with a comma separator, e.g., `--debug-flag=RubySequencer,ProtocolTrace`.

Additional debugging statements: `DPRINTF(<debug-flag-name>,"Statement");`. The statement can be any valid statement for `printf` function.

```C++
DPRINTF(RubySequencer, "Method call in Sequencer");
// Print addr in hexadecimal format and machine type as string
DPRINTF(RubySequencer, "Block Address: %#x by %s",addr, machine_type);
```

```shell
bear -a -vv scons build/X86_MESI_Two_Level_Extended/gem5.opt -j4 --default=X86 PROTOCOL=MESI_Two_Level_Extended SLICC_HTML=True; build/X86_MESI_Two_Level_Extended/gem5.opt --debug-flag=RubyNetwork,RubyPort,RubyQueue,RubySequencer,ProtocolTrace,Exec,DRAM --debug-file=DebugLog --debug-start=450000000 --debug-end=540000000 --outdir=./output configs/example/se.py --ruby --cpu-type=DerivO3CPU --mem-size=8GB --num-cpus=5 --cacheline_size=64 --l1d_assoc=8 --l1d_size=32kB --l1i_assoc=8 --l1i_size=32kB --l2_assoc=16 --l2_size=16MB --cmd="/data/swarnendu/false-sharing-benchmarks/microbenchmarks/true-sharing-dynamic" --options=""
```

There is no need for the `RubySequencer` flag if we provide `Ruby` as debug flag. `Ruby` is a composite flag for all ruby structures.

Refreshing protocol races:

- An core C0 having block in E state receives INV.<br>
  **Scenario:**<br>

  - C0 sends GETS for block, Direcotry provide exclusive data (M).
  - C0 receives exclusive response, state transition to E.
  - C1 sends GETS for block. Direcotry forward to C0 and transition to S state.
  - C2 sends a GETX for block. Directory sends out INV for C0 and C1.
  - The invalidation by dirctory reaches C0 ahead of forwarded request by C1.
  - C0 do not issue a write back for block
  - C0 will receive the forwarded request later and then issue the writeback

- On termination of privatization.<br>
  **Scenario I:**<br>

  - C0 and C1 has block in PRV state. Dir state is also PRV.
  - C2 issues a GETS for block, Dir perform conflict check, send Data response.
  - C3 issues a GETX request for block, Dir identifies conflict terminate privatization and send out INV to C0, C1, C2.
  - INV arrives before the GETS response at C2.
    **Solution:**<br>
  - C2 should immediately respond to INV and on arrival of data block follow gem5.

  **Scenario II:**<br>

  - C0 and C1 has block in PRV state. Dir state is also PRV.
  - C2 evicts the block and issue PUTX msg.
  - C3 issues a GETX request for block, Dir identifies conflict terminate privatization and send out INV to C0, C1, C2.
  - Dir receives the PUTX and sends out ACK to C2.
  - C2 issues a GETS for block.
  - INV arrives before the GETS response at C2.
    **Solution:**<br>
  - C2 should immediately respond to INV and on arrival of data block follow gem5.

##### Changing relplacement policy:

- No command line options available

- Obly through configuration file
- File `configs/ruby/FS_MESI.py` update the `replacement_policy` variable for each cache object with desired replacement policy.

---

##### FULL SYSTEM ERROR:

- [PerfKvmCounter Error](https://www.mail-archive.com/gem5-users@gem5.org/msg17340.html) If getting `panic: PerfKvmCounter::attach recieved error EACCESS`
- [X86KvmCPU fails](https://www.mail-archive.com/gem5-users@gem5.org/msg18487.html) Error: X86KvmCPU fails -- reason code 0x80000021
  Need to add the patch provided in the (https://gem5-review.googlesource.com/c/public/gem5/+/12278/5/src/arch/x86/fs_workload.cc#b207) to the `src/arch/x86/fs_workload.cc` file. Earlier the code was part of the `src/arch/x86/system.cc` file.
  - Refer the link(https://gem5-users.gem5.narkive.com/8DBihuUx/running-fs-py-with-x86kvmcpu-failed) for solution
    Also add these two patches:
  - https://gem5-review.googlesource.com/c/public/gem5/+/7362
  - https://gem5-review.googlesource.com/c/public/gem5/+/7361 (Adding this should suffice)
  - After adding patch 7361, `MiscReg` definition issue, replace it by `RegVal`
- Set `perf_event_paranoid` variable to -1 using command `echo -1 > /proc/sys/kernel/perf_event_paranoid`. Verify by running `cat /proc/sys/kernel/perf_event_parnoid`. [Link](https://www.intel.com/content/www/us/en/docs/vtune-profiler/cookbook/2023-0/profiling-hardware-without-sampling-drivers.html)
-

#### Full System: Image Mount/Unmount steps:

##### Creating a image file:

- Install [qemu](https://linuxize.com/post/how-to-install-kvm-on-ubuntu-18-04/)(easier to use), else use the steps listed in [create the image file](https://www.gem5.org/documentation/general_docs/fullsystem/disks):

  - Creating a image using [qemu](https://qemu-project.gitlab.io/qemu/system/images.html) <br>`qemu-img create test-img 24G`
  - Download the iso file from [Ubuntu releases](https://old-releases.ubuntu.com/releases/). Prefer 18.04.2 server image, desktop images does not work well with gem5.
  - Install the os into image file: <br> `sudo qemu-system-x86_64 -drive file=test-img,format=raw -cdrom ubuntu-18.04.2-server-amd64.iso -m 2048 -enable-kvm -boot d` <br>Do not use logical partition(as does not work with gem5), and create a single partition. If using a new ubuntu release check installation steps listed [here](https://ubuntu.com/server/docs/install/storage).
  - Boot the image <br>`sudo qemu-system-x86_64 -drive file=test-img,format=raw -cdrom ubuntu-18.04.2-server-amd64.iso -m 2048 -enable-kvm`
  - To enable the clipboard sharing between host and quest [stack-overflow](https://askubuntu.com/questions/858649/how-can-i-copypaste-from-the-host-to-a-kvm-guest)
    <br>`sudo apt install spice-vdagent`
  - Install the software/application of your choice.
  - `fdisk -l test-img`: gives the list of the partition in the image file.

  ```
  Disk test-img: 24 GiB, 25769803776 bytes, 50331648 sectors
  Units: sectors of 1 * 512 = 512 bytes
  Sector size (logical/physical): 512 bytes / 512 bytes
  I/O size (minimum/optimal): 512 bytes / 512 bytes
  Disklabel type: dos
  Disk identifier: 0x8beedf8b

  Device     Boot Start      End  Sectors Size Id Type
  test-img1  *     2048 50329599 50327552  24G 83 Linux
  ```

  Here, note the `Start` field for the partition marked with "\*" for boot. Calculate <offset> as _512\*Start_, not required if `Start` is 0

- `losetup -f`: find the first available loop-back device, e.g., /dev/loopXX
- sudo losetup -o <offset> /dev/loopXX test-img
- `sudo mount /dev/loopXX /mnt`
  After this you can view the content of `test-img` by `ls /mnt`

### To update the image file/ install new application/dependency( if do not prefer using qemu)

After mounting the image to loopback device

- sudo /bin/mount/ -o bind /sys /mnt/sys
- sudo /bin/mount/ -o bind /dev /mnt/dev
- sudo /bin/mount/ -o bind /proc /mnt/proc
- `sudo /usr/sbin/chroot mnt /bin/bash` to get root access in the mounted image
  <br>After update perform unmounting to remove the binding:
  '''
- sudo /bin/umount mnt/sys
- sudo /bin/umount mnt/proc
- sudo /bin/umount mnt/dev
- sudo umount /mnt
  '''

#### Install spec iso

- Copy the `config/flags/gcc.xml` to `config/flags/prospar.xml`. <br>
  Add a new flag to disable the [PIE](https://youtu.be/anRPs_owfwk?t=881).
-

#### Using Comm Monitor Object:

- https://www.youtube.com/live/TeHKMVOWUAY?feature=share&t=3639

#### Instruction squash code:

- src/cpu/o3/decode_impl.hh: (DefaultDecode<Impl>::squash(const DynInstPtr &inst, ThreadID tid))
  References

- Refer the [MOESI prime](https://dl.acm.org/doi/pdf/10.1145/3470496.3527427) for benchmark run status

- <https://nitish2112.github.io/post/statistics-gem5/>
- [MESI Protocol Description](http://www.gem5.org/documentation/general_docs/ruby/MESI_Two_Level/)
- [Designing a custom protocol](http://www.gem5.org/documentation/learning_gem5/part3/cache-declarations/)
- <http://learning.gem5.org/book/part1/cache_config.html>
- <http://learning.gem5.org/book/part2/helloobject.html>
- [Creating own disk image for FS simulation](http://www.lowepower.com/jason/creating-disk-images-for-gem5.html)
- [Setting-up Gem5 for Full system simulation using QEMU](http://www.lowepower.com/jason/setting-up-gem5-full-system.html)
- [For kernel .config file: use the one provided by Jason Lowe-Power in his blog post(Step 4)](http://www.lowepower.com/jason/setting-up-gem5-full-system.html)
- [Pre-built binaries for Gem5 for x86](http://www.m5sim.org/Download)
- <https://vlsiarch.ecen.okstate.edu/gem5/>
  <<<<<<< Updated upstream
- [Gem5 FAQs](http://old.gem5.org/Frequently_Asked_Questions.html#How_many_CPUs_can_M5_run.3F)
- [Resetting gem5 stats](https://www.gem5.org/documentation/general_docs/m5ops/)
- Adding new message and message type: Network.cc

- [Gem5-V20 Parsec Resources](https://gem5.googlesource.com/public/gem5-resources/+/refs/tags/v20.0.0.3)
  - our code compitable with version 20.0.0.3
- [linux-kernel](https://gem5.googlesource.com/public/gem5-resources/+/refs/tags/v20.0.0.3/src/linux-kernel/)
  Different pre-built binaries can be downloaded by replacing version from link(http://dist.gem5.org/dist/v20-1/kernels/x86/static/vmlinux-4.19.83) instead of 4.19.83 put required version.
- [Tutorial for gem5-v21](https://ucdavis365-my.sharepoint.com/:p:/g/personal/jlowepower_ucdavis_edu/EWxNhA79fP1Fk4byc0kfPK4BQDDTHdeNGA9p7SkAzvVO-Q?e=Y8aKMv&action=embedview&wdbipreview=true)

### Adding a custom stats in gem5:

- [Custom stats in GEM5](https://nitish2112.github.io/post/statistics-gem5/)
- Search using [Bard](bard.google.com) was also useful, it gives example to add a custom stats.

#### Perf C2C:

- [A good read for identifying the false sharing in multi-threaded applications](https://joemario.github.io/blog/2016/09/01/c2c-blog/)

### Setting up Feather

- Building kernel

  - Copy a existing config file into `.config` file in the directory of kernel source code.
  - `make -j<num_cores>`

- Building libmonitor

  - ./configure --prefix=/path/to/install
  - make
  - make install
  - Install the library to `/usr/local/lib`

- Building hpctoolkit-externals

  - cd externals
  - ./configure
  - make all
  - make distclean

- Buidling spack:

  - git clone -c feature.manyFiles=true https://github.com/spack/spack.git
  - cd spack/bin
  - ./spack install libelf

- Building hpctoolkit: - build Spack
- Gem5 FAQs <http://old.gem5.org/Frequently_Asked_Questions.html#How_many_CPUs_can_M5_run.3F>
