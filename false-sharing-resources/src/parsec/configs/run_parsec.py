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

""" Script to run PARSEC benchmarks with gem5.
    The script expects kernel, diskimage, cpu (kvm or timing),
    benchmark, benchmark size, and number of cpu cores as arguments.
    This script is best used if your disk-image has workloads tha have
    ROI annotations compliant with m5 utility. You can use the script in
    ../disk-images/parsec/ with the parsec-benchmark repo at
    https://github.com/darchr/parsec-benchmark.git to create a working
    disk-image for this script.
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


def writeBenchScript(dir, bench, size, num_cpus):
    """
    This method creates a script in dir which will be eventually
    passed to the simulated system (to run a specific benchmark
    at bootup).
    """
    # FalseSharing: simulating 8 core so run with fix 4 thread
    file_name = '{}/run_{}'.format(dir, bench)
    bench_file = open(file_name, 'w+')
    bench_file.write('cd /home/gem5/parsec-benchmark\n')
    bench_file.write('source env.sh\n')
    bench_file.write('taskset -c 1,2,3,4,5 parsecmgmt -a run -p \
            {} -c gcc-hooks -i {} -n 4\n'.format(bench, size))

    # sleeping for sometime makes sure
    # that the benchmark's output has been
    # printed to the console
    bench_file.write('sleep 10 \n')
    bench_file.write('m5 exit \n')
    bench_file.close()
    return file_name


if __name__ == "__m5_main__":
    (opts, args) = parse_arguments()

    kernel = opts.kernel
    disk = opts.disk
    cpu_type = opts.cpu_type
    num_cpus = opts.num_cpus
    mem_sys = opts.mem_sys
    benchmark = opts.benchmark
    benchmark_size = opts.size

    if not cpu_type in ['kvm', 'timing']:
        m5.fatal("cpu not supported")

    # create the system we are going to simulate
    # system = MySystem(kernel, disk, int(num_cpus), opts, no_kvm=False)
    if (mem_sys == "classic"):
        system = MySystem(kernel, disk, num_cpus, opts, no_kvm=False)
    elif (mem_sys == "MESI_Two_Level" or "FS_MESI" or "FS_MESI_DETECTION" or "MESI_Nonblocking" or
          "FS_MESI_Blocking" or "FS_MESI_DETECTION_Blocking"):
        system = MyRubySystem(kernel, disk, num_cpus, opts)

    # Exit from guest on workbegin/workend
    system.exit_on_work_items = True

    # Create and pass a script to the simulated system to run the reuired
    # benchmark
    system.readfile = writeBenchScript(
        m5.options.outdir, benchmark, benchmark_size, num_cpus)

    # set up the root SimObject and start the simulation
    root = Root(full_system=True, system=system)

    if system.getHostParallel():
        # Required for running kvm on multiple host cores.
        # Uses gem5's parallel event queue feature
        # Note: The simulator is quite picky about this number!
        root.sim_quantum = int(1e9)  # 1 ms

    # needed for long running jobs
    m5.disableAllListeners()

    # instantiate all of the objects we've created above
    m5.instantiate()

    globalStart = time.time()

    print("Running the simulation")
    print("Using cpu: {}".format(cpu_type))

    start_tick = m5.curTick()
    end_tick = m5.curTick()
    start_insts = system.totalInsts()
    end_insts = system.totalInsts()
    m5.stats.reset()

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
