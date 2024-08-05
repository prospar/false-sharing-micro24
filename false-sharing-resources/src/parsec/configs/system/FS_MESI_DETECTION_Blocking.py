# Copyright (c) 2020 The Regents of the University of California.
# All Rights Reserved
#
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


""" This file creates a set of Ruby caches for the MESI TWO Level protocol
This protocol models two level cache hierarchy. The L1 cache is split into
instruction and data cache.
This system support the memory size of up to 3GB.
"""


import math

from m5.defines import buildEnv
from m5.util import fatal, panic

from m5.objects import *


class FSMESIDetectionBlockingCache(RubySystem):

    def __init__(self):
        if buildEnv['PROTOCOL'] != 'FS_MESI_DETECTION_Blocking':
            fatal("This system assumes FS_MESI_DETECTION Blocking protocol!")

        super(FSMESIDetectionBlockingCache, self).__init__()

        self._numL2Caches = 8  # original value was 8

    def setup(self, system, cpus, mem_ctrls, dma_ports, iobus, options):
        """Set up the Ruby cache subsystem. Note: This can't be done in the
           constructor because many of these items require a pointer to the
           ruby system (self). This causes infinite recursion in initialize()
           if we do this in the __init__.
        """
        # Ruby's global network.
        # self.network = MyNetwork(self)
        self.network = RingNetwork(self)

        # MESI_Two_Level example uses 5 virtual networks
        self.number_of_virtual_networks = 5
        self.network.number_of_virtual_networks = 5

        # There is a single global list of all of the controllers to make it
        # easier to connect everything to the global network. This can be
        # customized depending on the topology/network requirements.
        # L1 caches are private to a core, hence there are one L1 cache per CPU core.
        # The number of L2 caches are dependent to the architecture.
        self.controllers = \
            [L1Cache(system, self, cpu, self._numL2Caches, options) for cpu in cpus] + \
            [L2Cache(system, self, self._numL2Caches, options) for num in range(self._numL2Caches)] + \
            [DirController(self, system.mem_ranges, mem_ctrls)] + \
            [DMAController(self) for i in range(len(dma_ports))]

        for i in range(len(self.controllers)):
            self.controllers[i].clk_domain = cpus[0].clk_domain

        # Create one sequencer per CPU and dma controller.
        # Sequencers for other controllers can be here here.
        self.sequencers = [RubySequencer(version=i,
                                         # I/D cache is combined and grab from ctrl
                                         icache=self.controllers[i].L1Icache,
                                         dcache=self.controllers[i].L1Dcache,
                                         clk_domain=self.controllers[i].clk_domain,
                                         pio_master_port=iobus.slave,
                                         mem_master_port=iobus.slave,
                                         pio_slave_port=iobus.master
                                         ) for i in range(len(cpus))] + \
            [DMASequencer(version=i,
                          slave=port)
             for i, port in enumerate(dma_ports)
             ]
        for i in range(len(self.sequencers)):
            self.sequencers[i].clk_domain = cpus[0].clk_domain

        for i, c in enumerate(self.controllers[:len(cpus)]):
            c.sequencer = self.sequencers[i]
            c.sequencer.clk_domain = cpus[i].clk_domain

        # Connecting the DMA sequencer to DMA controller
        for i, d in enumerate(self.controllers[-len(dma_ports):]):
            i += len(cpus)
            d.dma_sequencer = self.sequencers[i]

        self.num_of_sequencers = len(self.sequencers)

        # Create the network and connect the controllers.
        # NOTE: This is quite different if using Garnet!
        self.network.connectControllers(self.controllers, options)
        self.network.setup_buffers()

        # Set up a proxy port for the system_port. Used for load binaries and
        # other functional-only things.
        self.sys_port_proxy = RubyPortProxy()
        system.system_port = self.sys_port_proxy.slave
        self.sys_port_proxy.pio_master_port = iobus.slave

        # Connect the cpu's cache, interrupt, and TLB ports to Ruby
        for i, cpu in enumerate(cpus):
            cpu.icache_port = self.sequencers[i].slave
            cpu.dcache_port = self.sequencers[i].slave
            cpu.createInterruptController()
            isa = buildEnv['TARGET_ISA']
            if isa == 'x86':
                cpu.interrupts[0].pio = self.sequencers[i].master
                cpu.interrupts[0].int_master = self.sequencers[i].slave
                cpu.interrupts[0].int_slave = self.sequencers[i].master
            if isa == 'x86' or isa == 'arm':
                cpu.itb.walker.port = self.sequencers[i].slave
                cpu.dtb.walker.port = self.sequencers[i].slave


class L1Cache(L1Cache_Controller):

    _version = 0

    @classmethod
    def versionCount(cls):
        cls._version += 1  # Use count for this particular type
        return cls._version - 1

    def __init__(self, system, ruby_system, cpu, num_l2Caches, options):
        """Creating L1 cache controller. Consist of both instruction
           and data cache. The size of data cache is 512KB and
           8-way set associative. The instruction cache is 32KB,
           2-way set associative.
        """
        super(L1Cache, self).__init__()

        self.version = self.versionCount()
        block_size_bits = int(math.log(system.cache_line_size, 2))
        l1i_size = '32kB'
        l1i_assoc = '8'
        l1d_size = '32kB'
        l1d_assoc = '8'
        # This is the cache memory object that stores the cache data and tags
        self.L1Icache = RubyCache(size=options.l1i_size,
                                  assoc=options.l1i_assoc,
                                  start_index_bit=block_size_bits,
                                  is_icache=True,
                                  dataAccessLatency=3, # 0.749 ns
                                  tagAccessLatency=1) # 0.21 ns
        self.L1Dcache = RubyCache(size=options.l1d_size,
                                  assoc=options.l1d_assoc,
                                  start_index_bit=block_size_bits,
                                  is_icache=False,
                                  dataAccessLatency=3, # 0.91 ns
                                  tagAccessLatency=1) # 0.217 ns
        self.l2_select_num_bits = int(math.log(num_l2Caches, 2))
        self.clk_domain = cpu.clk_domain
        self.prefetcher = RubyPrefetcher()
        self.send_evictions = self.sendEvicts(cpu)
        self.transitions_per_cycle = 4
        self.enable_prefetch = False
        self.ruby_system = ruby_system
        self.connectQueues(ruby_system)
        self.report_pc = options.report_pc
        # ADD tracking_width param, options.l1d_assoc, options.size_own
        self.OptionalOwnAccessMetadata = FSOptionalPerCoreState(
            tracking_width=options.tracking_width,
            assoc_own=options.l1d_assoc,
            size_own=options.size_own,
            offset_bit_index=block_size_bits)

    def getBlockSizeBits(self, system):
        bits = int(math.log(system.cache_line_size, 2))
        if 2**bits != system.cache_line_size.value:
            panic("Cache line size not a power of 2!")
        return bits

    def sendEvicts(self, cpu):
        """True if the CPU model or ISA requires sending evictions from caches
           to the CPU. Two scenarios warrant forwarding evictions to the CPU:
           1. The O3 model must keep the LSQ coherent with the caches
           2. The x86 mwait instruction is built on top of coherence
           3. The local exclusive monitor in ARM systems
        """
        if type(cpu) is DerivO3CPU or \
           buildEnv['TARGET_ISA'] in ('x86', 'arm'):
            return True
        return False

    def connectQueues(self, ruby_system):
        """Connect all of the queues for this controller.
        """
        self.mandatoryQueue = MessageBuffer()
        self.requestFromL1Cache = MessageBuffer()
        self.requestFromL1Cache.master = ruby_system.network.slave
        self.responseFromL1Cache = MessageBuffer()
        self.responseFromL1Cache.master = ruby_system.network.slave
        self.unblockFromL1Cache = MessageBuffer()
        self.unblockFromL1Cache.master = ruby_system.network.slave

        self.optionalQueue = MessageBuffer()

        self.requestToL1Cache = MessageBuffer()
        self.requestToL1Cache.slave = ruby_system.network.master
        self.responseToL1Cache = MessageBuffer()
        self.responseToL1Cache.slave = ruby_system.network.master


class L2Cache(L2Cache_Controller):

    _version = 0

    @classmethod
    def versionCount(cls):
        cls._version += 1  # Use count for this particular type
        return cls._version - 1

    def __init__(self, system, ruby_system, num_l2Caches, options):

        super(L2Cache, self).__init__()

        self.version = self.versionCount()
        # This is the cache memory object that stores the cache data and tags
        self.L2cache = RubyCache(size=options.l2_size,
                                 assoc=options.l2_assoc,
                                 start_index_bit=self.getBlockSizeBits(system, num_l2Caches),
                                 dataAccessLatency=8, # 1.522 ns
                                 # FalseSharing adding 2 cycle to data latency as default
                                 # behavior is parallel access
                                 tagAccessLatency=2) # 0.4693 ns

        self.transitions_per_cycle = '4'
        self.ruby_system = ruby_system
        self.connectQueues(ruby_system)
        self.report_pc = options.report_pc
        self.FSGlobalACTData = FSGlobalACT(global_act_size=options.global_act_size,
                                           tracking_width=options.tracking_width,
                                           assoc_act=options.l2_assoc,
                                           offset_bit_index=6,
                                           reset_interval=10000,
                                           opt_readers=options.opt_readers,
                                           report_pc=options.report_pc)
        self.inv_threshold = options.inv_threshold
        self.fetch_threshold = options.fetch_threshold
        self.hys_threshold = 3  # options.hys_threshold
        self.saturation_threshold = 2*options.inv_threshold

    def getBlockSizeBits(self, system, num_l2caches):
        l2_bits = int(math.log(num_l2caches, 2))
        bits = int(math.log(system.cache_line_size, 2)) + l2_bits
        return bits

    def connectQueues(self, ruby_system):
        """Connect all of the queues for this controller.
        """
        self.DirRequestFromL2Cache = MessageBuffer()
        self.DirRequestFromL2Cache.master = ruby_system.network.slave
        self.L1RequestFromL2Cache = MessageBuffer()
        self.L1RequestFromL2Cache.master = ruby_system.network.slave
        self.responseFromL2Cache = MessageBuffer()
        self.responseFromL2Cache.master = ruby_system.network.slave
        self.unblockToL2Cache = MessageBuffer()
        self.unblockToL2Cache.slave = ruby_system.network.master
        self.L1RequestToL2Cache = MessageBuffer()
        self.L1RequestToL2Cache.slave = ruby_system.network.master
        self.responseToL2Cache = MessageBuffer()
        self.responseToL2Cache.slave = ruby_system.network.master


class DirController(Directory_Controller):

    _version = 0

    @classmethod
    def versionCount(cls):
        cls._version += 1  # Use count for this particular type
        return cls._version - 1

    def __init__(self, ruby_system, ranges, mem_ctrls):
        """ranges are the memory ranges assigned to this controller.
        """
        if len(mem_ctrls) > 1:
            panic("This cache system can only be connected to one mem ctrl")
        super(DirController, self).__init__()
        self.version = self.versionCount()
        self.addr_ranges = ranges
        self.ruby_system = ruby_system
        self.directory = RubyDirectoryMemory()
        # Connect this directory to the memory side.
        self.memory = mem_ctrls[0].port
        self.connectQueues(ruby_system)

    def connectQueues(self, ruby_system):
        self.requestToDir = MessageBuffer()
        self.requestToDir.slave = ruby_system.network.master
        self.responseToDir = MessageBuffer()
        self.responseToDir.slave = ruby_system.network.master
        self.responseFromDir = MessageBuffer()
        self.responseFromDir.master = ruby_system.network.slave
        self.requestToMemory = MessageBuffer()
        self.responseFromMemory = MessageBuffer()


class DMAController(DMA_Controller):

    _version = 0

    @classmethod
    def versionCount(cls):
        cls._version += 1  # Use count for this particular type
        return cls._version - 1

    def __init__(self, ruby_system):
        super(DMAController, self).__init__()
        self.version = self.versionCount()
        self.ruby_system = ruby_system
        self.connectQueues(ruby_system)

    def connectQueues(self, ruby_system):
        self.mandatoryQueue = MessageBuffer()
        self.responseFromDir = MessageBuffer(ordered=True)
        self.responseFromDir.slave = ruby_system.network.master
        self.requestToDir = MessageBuffer()
        self.requestToDir.master = ruby_system.network.slave


class MyNetwork(SimpleNetwork):
    """A simple point-to-point network. This doesn't not use garnet.
    """

    def __init__(self, ruby_system):
        super(MyNetwork, self).__init__()
        self.netifs = []
        self.ruby_system = ruby_system

    def connectControllers(self, controllers):
        """Connect all of the controllers to routers and connec the routers
           together in a point-to-point network.
        """
        # MESH
        # FalseSharing: later add options,num_cpus and options.mesh_rows
        num_routers = 8  # options.num_cpus
        num_rows = 2  # options.mesh_rows

        link_latency = 1  # options.link_latency # used by simple and garnet
        router_latency = 3  # options.router_latency # only used by garnet

        cntrls_per_router, remainder = divmod(len(controllers), num_routers)
        assert(num_rows > 0 and num_rows <= num_routers)
        num_columns = int(num_routers / num_rows)
        assert(num_columns*num_rows == num_routers)

        # contrast to simple network uses only Switch
        self.routers = [Switch(router_id=i, latency=router_latency)
                        for i in range(num_routers)]

        link_count = 0
        network_nodes = []
        remainder_nodes = []
        for node_index in range(len(controllers)):
            if node_index < (len(controllers) - remainder):
                network_nodes.append(controllers[node_index])
            else:
                remainder_nodes.append(controllers[node_index])

        ext_links = []
        for (i, n) in enumerate(network_nodes):
            cntrl_level, router_id = divmod(i, num_routers)
            assert(cntrl_level < cntrls_per_router)
            ext_links.append(SimpleExtLink(link_id=link_count, ext_node=n,
                                           int_node=self.routers[router_id],
                                           latency=link_latency))
            link_count += 1

        for (i, node) in enumerate(remainder_nodes):
            assert(node.type == 'DMA_Controller')
            assert(i < remainder)
            ext_links.append(SimpleExtLink(link_id=link_count, ext_node=node,
                                           int_node=self.routers[0],
                                           latency=link_latency))
            link_count += 1

        self.ext_links = ext_links

        # Create the mesh links.
        int_links = []

        # East output to West input links (weight = 2)
        for row in range(num_rows):
            for col in range(num_columns):
                if (col + 1 < num_columns):
                    east_out = col + (row * num_columns)
                    west_in = (col + 1) + (row * num_columns)
                    int_links.append(SimpleIntLink(link_id=link_count,
                                                   src_node=self.routers[east_out],
                                                   dst_node=self.routers[west_in],
                                                   latency=link_latency,
                                                   weight=2))
                    link_count += 1

        # West output to East input links (weight = 1)
        for row in range(num_rows):
            for col in range(num_columns):
                if (col + 1 < num_columns):
                    east_in = col + (row * num_columns)
                    west_out = (col + 1) + (row * num_columns)
                    int_links.append(SimpleIntLink(link_id=link_count,
                                                   src_node=self.routers[west_out],
                                                   dst_node=self.routers[east_in],
                                                   latency=link_latency,
                                                   weight=1))
                    link_count += 1

        # North output to South input links (weight = 2)
        for col in range(num_columns):
            for row in range(num_rows):
                if (row + 1 < num_rows):
                    north_out = col + (row * num_columns)
                    south_in = col + ((row + 1) * num_columns)
                    int_links.append(SimpleIntLink(link_id=link_count,
                                                   src_node=self.routers[north_out],
                                                   dst_node=self.routers[south_in],
                                                   latency=link_latency,
                                                   weight=2))
                    link_count += 1

        # South output to North input links (weight = 2)
        for col in range(num_columns):
            for row in range(num_rows):
                if (row + 1 < num_rows):
                    north_in = col + (row * num_columns)
                    south_out = col + ((row + 1) * num_columns)
                    int_links.append(SimpleIntLink(link_id=link_count,
                                                   src_node=self.routers[south_out],
                                                   dst_node=self.routers[north_in],
                                                   latency=link_latency,
                                                   weight=2))
                    link_count += 1

        self.int_links = int_links


class RingNetwork(SimpleNetwork):
    """A simple ring network. This doesn't not use garnet.
    """

    def __init__(self, ruby_system):
        super(RingNetwork, self).__init__()
        self.netifs = []
        self.ruby_system = ruby_system

    def connectControllers(self, controllers, opts):
        """Connect controllers to router to form ring topology
        """

        # RING
        # two additional router one for memory and one for DMA
        # (2 dma controller and 1 directory controller )
        num_routers = opts.num_cpus + 2

        link_latency = opts.link_latency  # used by simple and garnet
        router_latency = opts.router_latency  # only used by garnet

        cntrls_per_router, remainder = divmod(
            len(controllers), opts.num_cpus)

        # assert( len(controllers) == num_routers)
        # num_columns = int(num_routers / num_rows)
        # assert(num_columns*num_rows == num_routers)

        network_nodes = []
        remainder_nodes = []
        for node_index in range(len(controllers)):
            if node_index < (len(controllers) - remainder):
                network_nodes.append(controllers[node_index])
            else:
                remainder_nodes.append(controllers[node_index])

        # Create one router/switch per controller in the system
        self.routers = [Switch(router_id=i, latency=router_latency)
                        for i in range(num_routers)]

        # Make a link from each controller to the router. The link goes
        # externally to the network.

        link_count = 0

        ext_links = []
        for (i, n) in enumerate(network_nodes):
            cntrl_level, router_id = divmod(i, num_routers)
            assert(cntrl_level < cntrls_per_router)
            ext_links.append(SimpleExtLink(link_id=link_count, ext_node=n,
                                           int_node=self.routers[router_id],
                                           latency=link_latency))
            link_count += 1

        # FalseSharing: Dir controller is attached to a separate controllers
        # and DMA controller is attached to a separate controllers
        # for (i, node) in enumerate(remainder_nodes):
        #     # print(f"Node type:{node.type}:{node}")
        #     # assert(node.type == 'DMA_Controller')
        #     assert(i < remainder)
        #     ext_links.append(SimpleExtLink(link_id=link_count, ext_node=node,
        #                                    int_node=self.routers[num_routers -
        #                                                          (remainder - i)],
        #                                    latency=link_latency))
        #     link_count += 1

        # Dir controller
        ext_links.append(SimpleExtLink(link_id=link_count, ext_node=remainder_nodes[0],
                                       int_node=self.routers[num_routers - 2],
                                       latency=link_latency))
        link_count += 1

        # DMA controller0
        ext_links.append(SimpleExtLink(link_id=link_count, ext_node=remainder_nodes[1],
                                       int_node=self.routers[num_routers - 1],
                                       latency=link_latency))
        link_count += 1
        # DMA controller1
        ext_links.append(SimpleExtLink(link_id=link_count, ext_node=remainder_nodes[2],
                                       int_node=self.routers[num_routers - 1],
                                       latency=link_latency))
        link_count += 1

        self.ext_links = ext_links

        # Create the mesh links.
        int_links = []

        # East output to West input links (weight = 2)
        for row in range(num_routers):
            east_out = row % (num_routers)
            west_in = (row + 1) % (num_routers)
            int_links.append(SimpleIntLink(link_id=link_count,
                                           src_node=self.routers[east_out],
                                           dst_node=self.routers[west_in],
                                           latency=link_latency,
                                           weight=1))
            link_count += 1

        # West output to East input links (weight = 1)
        for row in range(num_routers):
            east_in = row % (num_routers)
            west_out = (row + 1) % (num_routers)
            int_links.append(SimpleIntLink(link_id=link_count,
                                           src_node=self.routers[west_out],
                                           dst_node=self.routers[east_in],
                                           latency=link_latency,
                                           weight=1))
            link_count += 1

        self.int_links = int_links
