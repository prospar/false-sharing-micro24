"""
Testscript to test SimObject Extension
for OD Metadata and GlobalACT metadata

import m5
from m5.objects  import *

root = Root(full_system=False)

root.percoreOD = FSPerCoreOD()

m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))

System configuration: 4core, Private L1 cache and Shared L2 cache.

Refer configs/example/se.py or configs/part3/simple_ruby.py: for multiple CPU

Useful Command line options:(Refer /configs/common/SimpleOpts.py)
----------------------------------------------------------------------

--cmd       specify the binary to execute

--options   input for executable binary given as string " "

--output    redirect Output to file

--errout    redirect error output

----------------------------------------
for network configuration : refer configs/ruby/Ruby.py 

First call to Ruby.py: Internally call <protocol_name>.py

For adding Options:
 1. define_options(parser): method in file
 2. Add a command line corresponding to file(e.g. --mem-org)
 3. Check, If --mem-org pass as cmd arg, call define_options 

Sample cmd to execute a test_suite benchmark:
build/X86_FS_MESI/gem5.opt --debug-flag=ODDebugFlag 
configs/false_sharing_project/runner_script.py --cpu-type="DerivO3CPU"
 --mem-size="4GB" --num-cpus=4 --caheline_size=64 
 --options="configs/false_sharing_project/test_suites/histogram/small.bmp 1"
  --l1i-assoc=8 --l1d-assoc=8

"""
from __future__ import print_function
from __future__ import absolute_import

# import the m5 (gem5) library created when gem5 is built
import m5
# import all of the SimObjects
from m5.objects import *
from optparse import OptionParser


m5.util.addToPath('../')

#from common.FileSystemConfig import config_filesystem
from ruby import Ruby
from common import Options
from common import SimpleOpts
#from common import ObjectList

parser = OptionParser()

'''
 use parser.parse_args() for using cmd flag from Options.py
 use SimpleOpts.parse_args() for using custom flag
'''
#refer configs/common/Options.py for adding command line
#define --num_cpus as   Options.py contains --num-cpus
parser.add_option('--num_cpus',default=2, type="int")
Options.addCommonOptions(parser)
Options.addSEOptions(parser)
#adding ruby flags
Ruby.define_options(parser)

(options, args) = parser.parse_args()

# create the system we are going to simulate
system = System()

# Set the clock fequency of the system (and all of its children)
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '2GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set up the system
system.mem_mode = 'timing'               # Use timing accesses
#need to create mem range used to create
system.mem_ranges = [AddrRange('8192MB')] # Create an address range

#creating a mutli-core system Refer configs/learning_gem5/part3/simple_ruby.py
cpu_count = options.num_cpus

'''
 Modify to use CPU type
'''
system.cpu = [DerivO3CPU() for i in range(cpu_count)]

# Create a DDR3 memory controller and connect it to the membus
#system.mem_ctrl = DDR3_1600_8x8()
#system.mem_ctrl.range = system.mem_ranges[0]

"""
 Setup system parameter first then call create_system function call
 CALL to "create_system()" defined in configs/ruby/Ruby.py
 Populate system entities like sequencer, caches, network, controller etc.
"""

Ruby.create_system(options, False, system, None, [],None)

#define interruptController after defining CPU sequencers.
for cpu in system.cpu:
    cpu.createInterruptController()
  
# create the interrupt controller for the CPU and connect to the membus
# refer tests/configs/simple-timing-ruby.py
# sequencer are stored in system.ruby._cpu_ports in conigs/ruby/Ruby.py
# Error related to IntMasterPort IntSlavePort as interrupt will be open.
for (i, cpu) in enumerate(system.cpu):
    # create the interrupt controller Defined in src/cpu/BaseCPU
    cpu.createInterruptController()
    # connect the cpu ports to the ruby cpu ports
    cpu.connectAllPorts(system.ruby._cpu_ports[i])

'''
    -------------Other Way to connect interrupts----------------
    ::::: > Refer configs/learning-gem5/part3/ruby_caches_MI_example.py
'''

# get ISA for the binary to run.
isa = str(m5.defines.buildEnv['TARGET_ISA']).lower()

# Run application and use the compiled ISA to find the binary
# grab the specific path to the binary
thispath = os.path.dirname(os.path.realpath(__file__))

#Later change to use --cmd flag
#for executing threads example
binary = os.path.join(thispath, '../../', 'tests/test-progs/threads/bin/',
                      isa, 'linux/threads')
#binary = os.path.join(thispath,'./test_suites/histogram/histogram')
# Create a process for a simple "multi-threaded" application
process = Process()

"""
 Set the command
 cmd is a list which begins with the executable (like argv)
    "--options" store cmd line option for binary
 Incase of mutliple process, separate the option for each file by ";"
 File Path: starting from configs/..../filename
"""
if options.options is not None:
    #print(options.options, " :: cmd option")
    process.cmd = [binary] + options.options.split()
else:
    process.cmd = [binary]

# Set the cpu to use the process as its workload and create thread contexts
for cpu in system.cpu:
    cpu.workload = process
    cpu.createThreads()

# set up the root SimObject and start the simulation
root = Root(full_system = False, system = system)
# instantiate all of the objects we've created above
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick {} because {}'.format(
         m5.curTick(), exit_event.getCause())
     )
