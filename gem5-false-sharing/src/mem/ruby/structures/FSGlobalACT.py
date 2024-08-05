# Refer src/learning_gem5/part2/HelloObject.py
'''
FalseSharing:  Implements the Global Act State, FIELDS, Structure
'''

from m5.params import *
from m5.SimObject import SimObject
# uncomment if using system var in Implementation
#from m5.objects import *


class FSGlobalACT(SimObject):
    type = 'FSGlobalACT'
    cxx_header = "mem/ruby/structures/FSGlobalACT.hh"

    global_act_size = Param.Int(0, "Number of entries in Global ACT table")
    tracking_width = Param.Int(1, "Each Access Metadata represent one byte or\
     n bytes of block")
    assoc_act = Param.Int(1, "Number of entry per set")
    offset_bit_index = Param.Int(6, "index start, default 6 for 64-byte line")
    reset_interval = Param.Tick("ticks to reset metadata")
    opt_readers = Param.Bool(False, "enable optimization for the reader\
    #  info or not")
    report_pc = Param.Bool(False, "report the PC of falsely shared line")
    #system = Param.System(Parent.any," System to which ACT Table belong")
