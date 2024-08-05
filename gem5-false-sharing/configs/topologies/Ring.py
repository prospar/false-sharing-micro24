# Copyright (c) 2011 Advanced Micro Devices, Inc.
#               2011 Massachusetts Institute of Technology
# All rights reserved.
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

from __future__ import print_function
from __future__ import absolute_import

from m5.params import *
from m5.objects import *

from topologies.BaseTopology import SimpleTopology

class Ring(SimpleTopology):
    description='Ring'

    def __init__(self, controllers):
        self.nodes = controllers

    def makeTopology(self, options, network, IntLink, ExtLink, Router):
        # nodes = self.nodes
        num_routers = options.num_cpus+1
        # default values for link latency and router latency.
        # Can be over-ridden on a per link/router basis
        link_latency = options.link_latency # used by simple and garnet
        router_latency = options.router_latency # only used by garnet

        # Create an individual router for each controller,
        # and connect all to all.
        # Since this is a high-radix router, router_latency should
        # accordingly be set to a higher value than the default
        # (which is 1 for mesh routers)
        cntrls_per_router, remainder = divmod(
            len(self.nodes), options.num_cpus)

        network_nodes = []
        remainder_nodes = []
        for node_index in range(len(self.nodes)):
            if node_index < (len(self.nodes) - remainder):
                network_nodes.append(self.nodes[node_index])
            else:
                remainder_nodes.append(self.nodes[node_index])

        routers = [Router(router_id=i, latency = router_latency) \
            for i in range(num_routers)]
        network.routers = routers
        print("fsdfsdfa")
        print(len(remainder_nodes))
        # Make a link from each controller to the router. The link goes
        # externally to the network.

        link_count = 0

        ext_links = []
        for (i, n) in enumerate(network_nodes):
            cntrl_level, router_id = divmod(i, num_routers)
            assert(cntrl_level < cntrls_per_router)
            ext_links.append(ExtLink(link_id=link_count, ext_node=n,
                                           int_node=routers[router_id],
                                           latency=link_latency))
            link_count += 1

        # Dir controller
        ext_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[0],
                                int_node=routers[num_routers - 1],
                                latency=link_latency))
        link_count += 1

        # # DMA controller0
        # ext_links.append(ExtLink(link_id=link_count,
        #                         ext_node=remainder_nodes[1],
        #                         int_node=routers[num_routers - 1],
        #                         latency=link_latency))
        # link_count += 1
        network.ext_links = ext_links



        int_links = []

        # East output to West input links (weight = 2)
        for row in range(num_routers):
            east_out = row % (num_routers)
            west_in = (row + 1) % (num_routers)
            int_links.append(SimpleIntLink(link_id=link_count,
                                           src_node=routers[east_out],
                                           dst_node=routers[west_in],
                                           latency=link_latency,
                                           weight=1))
            link_count += 1

        # West output to East input links (weight = 1)
        for row in range(num_routers):
            east_in = row % (num_routers)
            west_out = (row + 1) % (num_routers)
            int_links.append(SimpleIntLink(link_id=link_count,
                                           src_node=routers[west_out],
                                           dst_node=routers[east_in],
                                           latency=link_latency,
                                           weight=1))
            link_count += 1

        network.int_links = int_links
