"""
The configuration file for MESI modification performed to support DeFT
Refer : configs/ruby/MESI_Two_Level.py
"""


import math
import m5
from m5.objects import *
from m5.defines import buildEnv
from .Ruby import create_topology, create_directories
from .Ruby import send_evicts

#
# Declare caches used by the protocol
#


class L1Cache(RubyCache):
    dataAccessLatency = 3
    tagAccessLatency = 1
    pass


class L2Cache(RubyCache):
    dataAccessLatency = 8
    tagAccessLatency = 2
    pass


# FlaseSharing: moved to Options.py file
def define_options(parser):
    pass

def create_system(options, full_system, system, dma_ports, bootmem,
                  ruby_system):

    if buildEnv['PROTOCOL'] != 'FS_MESI_DETECTION':
        fatal("This script requires the FS_MESI_DETECTION\
         protocol to be built.")

    cpu_sequencers = []

    #
    # The ruby network creation expects the list of nodes in the system to be
    # consistent with the NetDest list.  Therefore the l1 controller nodes
    # must be listed before the directory nodes and directory nodes
    # before dma nodes, etc.
    #

    l1_cntrl_nodes = []
    l2_cntrl_nodes = []
    dma_cntrl_nodes = []

    #
    # Must create the individual controllers before the network to ensure the
    # controller constructors are called before the network constructor
    #
    l2_bits = int(math.log(options.num_l2caches, 2))
    block_size_bits = int(math.log(options.cacheline_size, 2))

    # FalseSharing: command line option
    tracking_width = int(options.tracking_width)
    inv_threshold = int(options.inv_threshold)
    fetch_threshold = int(options.fetch_threshold)
    # FalseSharing: AcT size 0: infinite size entry
    # FalseSharing: for infinite size AcT set is 1 and assoc is 0
    global_act_size = 0  # denotes num of entries in AcT table
    assoc_act = 0
    reset_interval = int(options.reset_tick)
    if options.global_act_size:
        global_act_size = int(options.global_act_size)
        assoc_act = int(options.assoc_act)

    for i in range(options.num_cpus):
        #
        # First create the Ruby objects associated with this cpu
        #
        l1i_cache = L1Cache(size=options.l1i_size,
                            assoc=options.l1i_assoc,
                            start_index_bit=block_size_bits,
                            is_icache=True)
        l1d_cache = L1Cache(size=options.l1d_size,
                            assoc=options.l1d_assoc,
                            start_index_bit=block_size_bits,
                            is_icache=False)
        # declaration of per_core_od object
        prefetcher = RubyPrefetcher()
        own_access_metadata = FSOptionalPerCoreState(
            tracking_width=tracking_width,
            assoc_own=options.l1d_assoc,
            size_own=options.size_own,
            offset_bit_index=block_size_bits)
        # eviction_metadata_table = FSEvictionMDCache(
        #     size_emd=8,
        #     assoc_emd=8,
        #     offset_bit_index=block_size_bits)
        # the ruby random tester reuses num_cpus to specify the
        # number of cpu ports connected to the tester object, which
        # is stored in system.cpu. because there is only ever one
        # tester object, num_cpus is not necessarily equal to the
        # size of system.cpu; therefore if len(system.cpu) == 1
        # we use system.cpu[0] to set the clk_domain, thereby ensuring
        # we don't index off the end of the cpu list.
        if len(system.cpu) == 1:
            clk_domain = system.cpu[0].clk_domain
        else:
            clk_domain = system.cpu[i].clk_domain
        # added a structure to store per_core_od metadata
        # FalseSharing: Constructor changes to introduce metadata extension
        l1_cntrl = L1Cache_Controller(
            version=i, L1Icache=l1i_cache,
            L1Dcache=l1d_cache,
            OptionalOwnAccessMetadata=own_access_metadata,
            # EvictionMDEntries=eviction_metadata_table,
            l2_select_num_bits=l2_bits,
            send_evictions=send_evicts(options),
            prefetcher=prefetcher,
            ruby_system=ruby_system,
            clk_domain=clk_domain,
            transitions_per_cycle=options.ports,
            enable_prefetch=False,
            report_pc=options.report_pc)

        cpu_seq = RubySequencer(version=i, icache=l1i_cache,
                                dcache=l1d_cache, clk_domain=clk_domain,
                                ruby_system=ruby_system)

        l1_cntrl.sequencer = cpu_seq
        exec("ruby_system.l1_cntrl%d = l1_cntrl" % i)

        # Add controllers and sequencers to the appropriate lists
        cpu_sequencers.append(cpu_seq)
        l1_cntrl_nodes.append(l1_cntrl)

        # Connect the L1 controllers and the network
        l1_cntrl.mandatoryQueue = MessageBuffer()
        l1_cntrl.requestFromL1Cache = MessageBuffer()
        l1_cntrl.requestFromL1Cache.master = ruby_system.network.slave
        l1_cntrl.responseFromL1Cache = MessageBuffer()
        l1_cntrl.responseFromL1Cache.master = ruby_system.network.slave
        l1_cntrl.unblockFromL1Cache = MessageBuffer()
        l1_cntrl.unblockFromL1Cache.master = ruby_system.network.slave

        l1_cntrl.optionalQueue = MessageBuffer()

        l1_cntrl.requestToL1Cache = MessageBuffer()
        l1_cntrl.requestToL1Cache.slave = ruby_system.network.master
        l1_cntrl.responseToL1Cache = MessageBuffer()
        l1_cntrl.responseToL1Cache.slave = ruby_system.network.master

    l2_index_start = block_size_bits + l2_bits

    for i in range(options.num_l2caches):
        #
        # First create the Ruby objects associated with this cpu
        #
        l2_cache = L2Cache(size=options.l2_size,
                           assoc=options.l2_assoc,
                           start_index_bit=l2_index_start)

        # Global Access Tracking table
        # FalseSharing: keeping associativity same as L2
        global_act = FSGlobalACT(tracking_width=tracking_width,
                                 global_act_size=global_act_size,
                                 assoc_act=assoc_act,
                                 offset_bit_index=block_size_bits,
                                 reset_interval=reset_interval,
                                 opt_readers=options.opt_readers,
                                 report_pc=options.report_pc)
        l2_cntrl = L2Cache_Controller(version=i,
                                      L2cache=l2_cache,
                                      FSGlobalACTData=global_act,
                                      transitions_per_cycle=options.ports,
                                      ruby_system=ruby_system,
                                      inv_threshold=inv_threshold,
                                      fetch_threshold=fetch_threshold,
                                      hys_threshold=3,
                                      saturation_threshold=(2*inv_threshold),
                                      report_pc=options.report_pc)
        # FSGlobalACTData=global_act,

        exec("ruby_system.l2_cntrl%d = l2_cntrl" % i)
        l2_cntrl_nodes.append(l2_cntrl)
        # FalseSharing: global access tracking metadata
        # l2_cntrl.FSGlobalACTData = FSGlobalACT()
        # Connect the L2 controllers and the network
        l2_cntrl.DirRequestFromL2Cache = MessageBuffer()
        l2_cntrl.DirRequestFromL2Cache.master = ruby_system.network.slave
        l2_cntrl.L1RequestFromL2Cache = MessageBuffer()
        l2_cntrl.L1RequestFromL2Cache.master = ruby_system.network.slave
        l2_cntrl.responseFromL2Cache = MessageBuffer()
        l2_cntrl.responseFromL2Cache.master = ruby_system.network.slave

        l2_cntrl.unblockToL2Cache = MessageBuffer()
        l2_cntrl.unblockToL2Cache.slave = ruby_system.network.master
        l2_cntrl.L1RequestToL2Cache = MessageBuffer()
        l2_cntrl.L1RequestToL2Cache.slave = ruby_system.network.master
        l2_cntrl.responseToL2Cache = MessageBuffer()
        l2_cntrl.responseToL2Cache.slave = ruby_system.network.master

    # Run each of the ruby memory controllers at a ratio of the frequency of
    # the ruby system
    # clk_divider value is a fix to pass regression.
    ruby_system.memctrl_clk_domain = DerivedClockDomain(
        clk_domain=ruby_system.clk_domain,
        clk_divider=3)

    mem_dir_cntrl_nodes, rom_dir_cntrl_node = create_directories(
        options, bootmem, ruby_system, system)
    dir_cntrl_nodes = mem_dir_cntrl_nodes[:]
    if rom_dir_cntrl_node is not None:
        dir_cntrl_nodes.append(rom_dir_cntrl_node)
    for dir_cntrl in dir_cntrl_nodes:
        # Connect the directory controllers and the network
        # dir_cntrl.FSGlobalACTData = FSGlobalACT()
        # Global ACT moved to LLC/or L2 in this case
        dir_cntrl.requestToDir = MessageBuffer()
        dir_cntrl.requestToDir.slave = ruby_system.network.master
        dir_cntrl.responseToDir = MessageBuffer()
        dir_cntrl.responseToDir.slave = ruby_system.network.master
        dir_cntrl.responseFromDir = MessageBuffer()
        dir_cntrl.responseFromDir.master = ruby_system.network.slave
        dir_cntrl.requestToMemory = MessageBuffer()
        dir_cntrl.responseFromMemory = MessageBuffer()

    for i, dma_port in enumerate(dma_ports):
        # Create the Ruby objects associated with the dma controller
        dma_seq = DMASequencer(version=i, ruby_system=ruby_system,
                               slave=dma_port)

        dma_cntrl = DMA_Controller(version=i, dma_sequencer=dma_seq,
                                   transitions_per_cycle=options.ports,
                                   ruby_system=ruby_system)

        exec("ruby_system.dma_cntrl%d = dma_cntrl" % i)
        dma_cntrl_nodes.append(dma_cntrl)

        # Connect the dma controller to the network
        dma_cntrl.mandatoryQueue = MessageBuffer()
        dma_cntrl.responseFromDir = MessageBuffer(ordered=True)
        dma_cntrl.responseFromDir.slave = ruby_system.network.master
        dma_cntrl.requestToDir = MessageBuffer()
        dma_cntrl.requestToDir.master = ruby_system.network.slave

    all_cntrls = l1_cntrl_nodes + \
        l2_cntrl_nodes + \
        dir_cntrl_nodes + \
        dma_cntrl_nodes

    # Create the io controller and the sequencer
    if full_system:
        io_seq = DMASequencer(version=len(dma_ports),
                              ruby_system=ruby_system)
        ruby_system._io_port = io_seq
        io_controller = DMA_Controller(version=len(dma_ports),
                                       dma_sequencer=io_seq,
                                       ruby_system=ruby_system)
        ruby_system.io_controller = io_controller

        # Connect the dma controller to the network
        io_controller.mandatoryQueue = MessageBuffer()
        io_controller.responseFromDir = MessageBuffer(ordered=True)
        io_controller.responseFromDir.slave = ruby_system.network.master
        io_controller.requestToDir = MessageBuffer()
        io_controller.requestToDir.master = ruby_system.network.slave

        all_cntrls = all_cntrls + [io_controller]

    ruby_system.network.number_of_virtual_networks = 3
    topology = create_topology(all_cntrls, options)
    return (cpu_sequencers, mem_dir_cntrl_nodes, topology)
