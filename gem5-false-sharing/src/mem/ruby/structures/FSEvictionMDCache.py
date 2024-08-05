# Refer src/learning_gem5/part2/HelloObject.py
'''
FalseSharing:  Implements the EvictionMD cache to handle outstanding
 MD eviction meessages
'''

from m5.params import *
from m5.SimObject import SimObject
# uncomment if using system var in Implementation
#from m5.objects import *


class FSEvictionMDCache(SimObject):
    type = 'FSEvictionMDCache'
    cxx_header = "mem/ruby/structures/FSEvictionMDCache.hh"

    size_emd = Param.Int(512, "Number of entries in the Eviction MD cache")
    assoc_emd = Param.Int(8, "Number of entry per set, keep it same as L1D")
    offset_bit_index = Param.Int(6, "index start, default 6 for 64-byte line")
    #system = Param.System(Parent.any," System to which ACT Table belong")
