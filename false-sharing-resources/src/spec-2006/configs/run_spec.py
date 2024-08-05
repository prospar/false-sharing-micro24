# -*- coding: utf-8 -*-
# Copyright (c) 2019 The Regents of the University of California.
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Jason Lowe-Power, Ayaz Akram, Hoa Nguyen

""" Script to run a SPEC benchmark in full system mode with gem5.

    Inputs:
    * This script expects the following as arguments:
        ** kernel:
                  This is a positional argument specifying the path to
                  vmlinux.
        ** disk:
                  This is a positional argument specifying the path to the
                  disk image containing the installed SPEC benchmarks.
        ** cpu:
                  This is a positional argument specifying the name of the
                  detailed CPU model. The names of the available CPU models
                  are available in the getDetailedCPUModel(cpu_name) function.
                  The function should be modified to add new CPU models.
                  Currently, the available CPU models are:
                    - kvm: this is not a detailed CPU model, ideal for testing.
                    - o3: DerivO3CPU.
                    - atomic: AtomicSimpleCPU.
                    - timing: TimingSimpleCPU.
        ** benchmark:
                  This is a positional argument specifying the name of the
                  SPEC benchmark to run. Most SPEC benchmarks are available.
                  Please follow this link to check the availability of the
                  benchmarks. The working benchmark matrix is near the end
                  of the page:
         (SPEC 2006) https://gem5art.readthedocs.io/en/latest/tutorials/spec2006-tutorial.html#appendix-i-working-spec-2006-benchmarks-x-cpu-model-table
         (SPEC 2017) https://gem5art.readthedocs.io/en/latest/tutorials/spec2017-tutorial.html#appendix-i-working-spec-2017-benchmarks-x-cpu-model-table
        ** size:
                  This is a positional argument specifying the size of the
                  benchmark. The available sizes are: ref, test, train.
"""
import errno
import os
import sys
import time
import m5
import m5.ticks
from m5.objects import *

import optparse

from system import *


def parse_arguments():
    parser = optparse.OptionParser(description="gem5 config to run GAPBS")
    parser.add_option("--kernel", type=str, help="Path to vmlinux")
    parser.add_option("--disk", type=str,
                      help="Path to the disk image containing GAPBS")
    parser.add_option("--cpu_type", type=str, help="Name of the detailed CPU")
    parser.add_option("--num_cpus", type=int, help="Number of CPUs")
    parser.add_option("--mem_sys", type=str,
                      help="Memory model, Classic or MI_example")
    parser.add_option("--benchmark", type=str,
                      help="Name of the benchmark")
    # FalseSharing: common options
    parser.add_option("--caches", action="store_true")
    parser.add_option("--l2cache", action="store_true")
    parser.add_option("--num-dirs", type=int, default=1)
    parser.add_option("--num-l2caches", type=int, default=1)
    parser.add_option("--num-l3caches", type=int, default=1)
    parser.add_option("--l1d_size", type="string", default="32kB")
    parser.add_option("--l1i_size", type="string", default="32kB")
    parser.add_option("--l2_size", type="string", default="16MB")
    parser.add_option("--ruby", action="store_true")
    # parser.add_option("--l3_size", type="string", default="16MB")
    parser.add_option("--l1d_assoc", type=int, default=8)
    parser.add_option("--l1i_assoc", type=int, default=8)
    parser.add_option("--l2_assoc", type=int, default=16)
    # parser.add_option("--l3_assoc", type=int, default=16)
    parser.add_option("--cacheline_size", type=int, default=64)
    # FalseSharing options
    parser.add_option("--inv_threshold", type=int, default=8,
                      help="number of invalidation message threshold")
    parser.add_option("--fetch_threshold", type=int, default=8,
                      help="number of fetch request threshold")
    parser.add_option("--global_act_size", type=int, default=128,
                      help="number of block entries in global act table")
    parser.add_option("--tracking_width", type=int, default=1,
                      help="access metadata represent per t bytes,\
                           default per 1 byte")
    parser.add_option("--size_own", type=int, default=512,
                      help="total entry in own access metadata,\
                           must equal to total cache block in L1D-cache")
    parser.add_option("--hys_threshold", type=int, default=3,
                      help="to prevent the repetetive privatization")
    parser.add_option("--opt_readers", action='store_true', default=False,
                      help="enable optimization for reader field in SAM table")
    parser.add_option("--report_pc", action="store_true", default=False,
                      help="enable reporting of inst involve in false sharing")
    # parsec_specific options
    parser.add_option("--no_prefetchers", default=False,
                      action="store_true",
                      help="Enable prefectchers on the caches")
    parser.add_option("--size", type=str, default="simsmall",
                      help="input size for parsec benchmarks")
    # adding new options for freq settings
    parser.add_option("--cpu_freq", type=str, default="3GHz",
                      help="frequency for the cpu clock")
    parser.add_option("--clk_freq", type=str, default="3GHz",
                      help="frequency for the system clock")
    parser.add_option("--dram_type", type=str, default="DDR4",
                      help="type of DRAM to use")
    parser.add_option("--router_latency", type=int, default=4,
                      help="routing latency for the network")
    parser.add_option("--link_latency", type=int, default=1,
                      help="routing latency for the network")
    # adding new options for MD comm and repetitive reporting in Detecet protocol
    parser.add_option("--enableMultipleReport", action="store_false", default=True,
                      help="allow repetitive reporting in Detect protocol")
    parser.add_option("--disableMDCommOpt", action="store_false", default=True,
                      help="disable MD communication in Detect protocol\
                        beyond detection")
    parser.add_option("--saturation_threshold", type=int, default=128,
                      help="set the saturation threshold for the fc/ic")
    return parser.parse_args()


def writeBenchScript(dir, benchmark_name, size, output_path, num_cpus):
    """
    This method creates a script in dir which will be eventually
    passed to the simulated system (to run a specific benchmark
    at bootup).
    """
    input_file_name = '{}run_{}'.format(dir, benchmark_name)
    with open(input_file_name, "w") as f:
        f.write('{} {} {}'.format(benchmark_name, size, output_path))
    return input_file_name


def getDetailedCPUModel(cpu_name):
    '''
    Return the CPU model corresponding to the cpu_name.
    '''
    available_models = {"kvm": X86KvmCPU,
                        "o3": DerivO3CPU,
                        "atomic": AtomicSimpleCPU,
                        "timing": TimingSimpleCPU
                        }
    try:
        available_models["FlexCPU"] = FlexCPU
    except NameError:
        # FlexCPU is not defined
        pass
    # https://docs.python.org/3/library/stdtypes.html#dict.get
    # dict.get() returns None if the key does not exist
    return available_models.get(cpu_name)


def create_system(linux_kernel_path, disk_image_path, detailed_cpu_model, memory_system, num_cpus, opts):
    # create the system we are going to simulate
    ruby_protocols = ["MI_example", "MESI_Two_Level", "MOESI_CMP_directory",
                      "FS_MESI", "MESI_Nonblocking", "FS_MESI_DETECTION", "FS_MESI_Blocking",
                      "FS_MESI_DETECTION_Blocking"]
    if memory_system == 'classic':
        system = MySystem(kernel=linux_kernel_path,
                          disk=disk_image_path,
                          num_cpus=num_cpus,  # run the benchmark in a single thread
                          no_kvm=False,
                          TimingCPUModel=detailed_cpu_model, opts=opts)
    elif memory_system in ruby_protocols:
        system = MyRubySystem(kernel=linux_kernel_path,
                              disk=disk_image_path,
                              num_cpus=num_cpus,  # run the benchmark in a single thread
                              mem_sys=memory_system,
                              TimingCPUModel=detailed_cpu_model, opts=opts)
    else:
        m5.fatal("Bad option for mem_sys, should be "
                 "{}, or 'classic'".format(', '.join(ruby_protocols)))

    # For workitems to work correctly
    # This will cause the simulator to exit simulation when the first work
    # item is reached and when the first work item is finished.
    system.exit_on_work_items = True
    # system.work_begin_exit_count = 1
    # system.work_end_exit_count = 1

    # set up the root SimObject and start the simulation
    root = Root(full_system=True, system=system)

    if system.getHostParallel():
        # Required for running kvm on multiple host cores.
        # Uses gem5's parallel event queue feature
        # Note: The simulator is quite picky about this number!
        root.sim_quantum = int(1e9)  # 1 ms

    return root, system


if __name__ == "__m5_main__":
    (opts, args) = parse_arguments()

    cpu_type = opts.cpu_type
    mem_sys = opts.mem_sys
    benchmark_name = opts.benchmark
    benchmark_size = opts.size
    linux_kernel_path = opts.kernel
    disk_image_path = opts.disk
    num_cpus = opts.num_cpus

    if not cpu_type in ['atomic', 'kvm', 'timing']:
        m5.fatal("cpu_type not supported")

    output_dir = os.path.join(m5.options.outdir, "custom_bm_logs")

    # Get the DetailedCPU class from its name
    detailed_cpu = getDetailedCPUModel(cpu_type)
    if detailed_cpu == None:
        print("'{}' is not define in the config script.".format(cpu_type))
        print("Change getDeatiledCPUModel() in run_spec.py "
              "to add more CPU Models.")
        exit(1)

    root, system = create_system(linux_kernel_path, disk_image_path,
                                 detailed_cpu, mem_sys, num_cpus, opts)

    # Create and pass a script to the simulated system to run the required
    # benchmark
    system.readfile = writeBenchScript(m5.options.outdir, benchmark_name,
                                       benchmark_size, output_dir, num_cpus)

    # needed for long running jobs
    m5.disableAllListeners()

    # instantiate all of the objects we've created above
    m5.instantiate()
    globalStart = time.time()
    # booting linux
    print("Booting Linux")

    start_tick = m5.curTick()
    end_tick = m5.curTick()
    start_insts = system.totalInsts()
    end_insts = system.totalInsts()
    m5.stats.reset()

    exit_event = m5.simulate()

    if exit_event.getCause() == "m5_exit instruction encountered":
        print("Booting done")
    exit_event = m5.simulate()
    if exit_event.getCause() == "workbegin":
        # Reached the start of ROI
        # start of ROI is marked by an
        # m5_work_begin() call
        print("Resetting stats at the start of ROI!")
        m5.stats.reset()
        start_tick = m5.curTick()
        start_insts = system.totalInsts()
        # switching to timing cpu if argument cpu == timing
        if cpu_type == 'timing':
            system.switchCpus(system.cpu, system.timingCpu)
    else:
        print("Unexpected termination of simulation!")
        print(exit_event.getCause())
        m5.stats.dump()
        end_tick = m5.curTick()
        end_insts = system.totalInsts()
        m5.stats.reset()
        print("Performance statistics:")

        print("Simulated time: %.2fs" % ((end_tick-start_tick)/1e12))
        print("Instructions executed: %d" % ((end_insts-start_insts)))
        print("Ran a total of", m5.curTick()/1e12, "simulated seconds")
        print("Total wallclock time: %.2fs, %.2f min" %
              (time.time()-globalStart, (time.time()-globalStart)/60))
        exit()

    # Simulate the ROI
    exit_event = m5.simulate()

    # Reached the end of ROI
    # Finish executing the benchmark with kvm cpu
    if exit_event.getCause() == "workend":
        # Reached the end of ROI
        # marked by an m5_work_end() call
        print("Dump stats at the end of the ROI!")
        m5.stats.dump()
        end_tick = m5.curTick()
        end_insts = system.totalInsts()
        m5.stats.reset()
        # switching to timing cpu if argument cpu == timing
        if cpu_type == 'timing':
            # system.switchCpus(system.timingCpu, system.cpu)
            print("Performance statistics:")

            print("Simulated time: %.2fs" % ((end_tick-start_tick)/1e12))
            print("Instructions executed: %d" % ((end_insts-start_insts)))
            print("Ran a total of", m5.curTick()/1e12, "simulated seconds")
            print("Total wallclock time: %.2fs, %.2f min" %
                  (time.time()-globalStart, (time.time()-globalStart)/60))
            exit()
    else:
        print("Unexpected termination of simulation!")
        print(exit_event.getCause())
        m5.stats.dump()
        end_tick = m5.curTick()
        end_insts = system.totalInsts()
        m5.stats.reset()
        print("Performance statistics:")

        print("Simulated time: %.2fs" % ((end_tick-start_tick)/1e12))
        print("Instructions executed: %d" % ((end_insts-start_insts)))
        print("Ran a total of", m5.curTick()/1e12, "simulated seconds")
        print("Total wallclock time: %.2fs, %.2f min" %
              (time.time()-globalStart, (time.time()-globalStart)/60))
        exit()

    # Simulate the remaning part of the benchmark
    exit_event = m5.simulate()
    print("Done with the simulation: {}".format(exit_event.getCause()))
    print()
    print("Performance statistics:")

    print("Simulated time in ROI: %.2fs" % ((end_tick-start_tick)/1e12))
    print("Instructions executed in ROI: %d" % ((end_insts-start_insts)))
    print("Ran a total of", m5.curTick()/1e12, "simulated seconds")
    print("Total wallclock time: %.2fs, %.2f min" %
          (time.time()-globalStart, (time.time()-globalStart)/60))
