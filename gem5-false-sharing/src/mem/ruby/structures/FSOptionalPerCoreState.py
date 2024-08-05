'''
Stores Optional PerCore Metadata
 Reduce update frequency to GLobal ACT state
'''

from m5.params import *
from m5.SimObject import SimObject


class FSOptionalPerCoreState(SimObject):
    type = 'FSOptionalPerCoreState'
    cxx_header = "mem/ruby/structures/FSOptionalPerCoreState.hh"

    tracking_width = Param.Int(1,"Each Access Metadata represent one byte or\
     n bytes of block")
    # FalseSharing: assoc of own metadata table same as L1D-cache
    # FalseSharing: default to 8 way
    assoc_own = Param.Int(8, "Associativity of own access metadata table")
    # Falsesharing: size denotes number of entries in table
    # FalseSharing: default to 512 for 8 way 32KB cache
    size_own = Param.Int(512, "Size of own access metadata table")
    offset_bit_index = Param.Int(6, "index start, default 6 for 64-byte line")

