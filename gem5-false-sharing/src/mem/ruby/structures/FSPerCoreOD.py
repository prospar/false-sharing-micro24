# Refer src/learning_gem5/part2/HelloObject.py
'''
FalseSharing:
Implements the Per Core Overlap Detection Structure
 - Similar to CacheMemory
 - Uses Private cache controller
 - Size and Associativity simialr to Cache Memory
Latency of OD table: Parallel Lookup (Hide latency )
Defining paramtype: refer src/python/m5/params.py


PerCoreOD:

Lookup similar to L1D cache:
Reuse:
 - mem/cache/tags

Define parameters:
 - Use inbuilt types from python/m5/params.py

- od_associativity: same as cores' private cache

- od_size: decide on runtime
    each OD_Entry will be 67 bits (1 PFSbit + 64 bit PC)

    #od_lookup = Param.Latency(2,"Cycle to lookup OD state")
od_size = Param.MemorySize("Size of metadata to store OD data")


Use 'cxx_class' if c++ class name different from python filename
Later Implementation:
 - pass from cache constructor:
    #od_associativity = Param.Int(1, "Associativity of OD table")
    #od_num_of_set = Param.Int(0,"Number of set in OD table")
    #od_size = Param.Int(0,"Number of Entries in OD table")
    -- od_size 0(Infinite buffer)
 - bank implementation
    #od_addrArrayBank = Param.Int(1,"Number of banks to store tag")
    #od_dataArrayBank = Param.Int(1,"Number of banks to store OD metadata")
    #system = Param.System(Parent.any,"System having OD state metdata")
    - use system.block-size for defining od_state_table size.
'''

from m5.params import *
#from m5.proxy import *
from m5.SimObject import SimObject
# uncomment if using system var in Implementation
#from m5.objects import *


class FSPerCoreOD(SimObject):
    type = 'FSPerCoreOD'
    cxx_header = "mem/ruby/structures/FSPerCoreOD.hh"
    od_size = Param.Int(0, "Size of OD metadata table")
    #system = Param.System(Parent.any,"TO access system blocksize for table
    # size")
