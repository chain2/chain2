#!/usr/bin/env python3
# Copyright (c) 2020 The chain2 developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from time import sleep

class ReorgResistanceTest (BitcoinTestFramework):
    def __init__(self):
        super().__init__()
        self.num_nodes = 4
        self.setup_clean_chain = True

    def setup_network(self):
        self.nodes = []
        self.is_network_split = False
        self.nodes.append(start_node(0, self.options.tmpdir, ["-debug=block"]))
        self.nodes.append(start_node(1, self.options.tmpdir, ["-debug=block"]))
        self.nodes.append(start_node(2, self.options.tmpdir, ["-debug=block"]))
        self.nodes.append(start_node(3, self.options.tmpdir, ["-debug=block"]))

    def run_test (self):
        # Node 0's active chain is two blocks atop genesis
        # Block                       B00<-B01<-B02
        # Penalized Work               2    2    2
        # Penalized Parent Chainwork   -    2    4

        activeforkstarttime = self.nodes[0].getblock(self.nodes[0].generate(1)[0])['time']
        node0tiphash = self.nodes[0].generate(1)[0]
        fork0chainwork = int(self.nodes[0].getblock(node0tiphash)['chainwork'], 16)
        assert_equal(fork0chainwork, 6)

        # Test 1

        # Node 1 builds a competing 3-block fork
        # Block                       B00<-B11<-B12<-B13
        # Penalized Work               2    0    0    0
        # Penalized Parent Chainwork   -    2    2    3

        self.nodes[1].setmocktime(activeforkstarttime)
        self.nodes[1].generate(3);
        node1tiphash = self.nodes[1].getbestblockhash()
        fork1chainwork = int(self.nodes[1].getblock(node1tiphash)['chainwork'], 16)
        assert_equal(fork1chainwork, 8)

        # Let node0 receive the 3-block chain 1800 seconds after the first would-be orphan's timestamp
        self.nodes[0].setmocktime(activeforkstarttime + 1800)
        connect_nodes(self.nodes[0],1)
        sleep(2)
        # Even though the fork has more parent chainwork, it is heavily penalized (late blocks valued
        # at 1/4 of actual work, rounding to 0 in this case) and not accepted by node0
        assert_equal(self.nodes[0].getbestblockhash(), node0tiphash)
        # node1 also does not switch to node0's chain
        assert_equal(self.nodes[1].getbestblockhash(), node1tiphash)

        # Test 2

        # Node 2 starts a competing fork with 3 blocks
        # Block                       B00<-B21<-B22<-B23
        # Penalized Work               2    1    1    1
        # Penalized Parent Chainwork   -    2    3    4

        self.nodes[2].setmocktime(activeforkstarttime)
        self.nodes[2].generate(3);
        node2tiphash = self.nodes[2].getbestblockhash()
        fork2chainwork = int(self.nodes[2].getblock(node2tiphash)['chainwork'], 16)
        assert_equal(fork2chainwork, 8)

        # Let node0 receive the 3-block fork 600 seconds after the first would-be orphan's timestamp
        self.nodes[0].setmocktime(activeforkstarttime + 600)
        connect_nodes(self.nodes[0],2)
        sleep(2)
        # The fork penalties reduce parent chainwork to 2+1+1, exactly node0's parent chainwork of 4
        # Devolving to the first-seen rule, node0 does not switch to node2's chain
        assert_equal(self.nodes[0].getbestblockhash(), node0tiphash)
        # node2 also does not switch to node0's chain
        assert_equal(self.nodes[2].getbestblockhash(), node2tiphash)

        # Test 3

        # Node 3 builds a competing 3-block fork with timestamps far in node2's future
        # (this is allowed in regtest, and should NOT affect penalization)
        # Block                       B00<-B31<-B32<-B33
        # Penalized Work               2    2    2    1
        # Penalized Parent Chainwork   -    2    4    6

        self.nodes[3].setmocktime(activeforkstarttime + 50000)
        self.nodes[3].generate(3);
        node3tiphash = self.nodes[3].getbestblockhash()
        fork3chainwork = int(self.nodes[3].getblock(node3tiphash)['chainwork'], 16)
        assert_equal(fork3chainwork, 8)

        # Let node0 receive the 3-block chain 599 seconds after the first would-be orphan's timestamp
        self.nodes[0].setmocktime(activeforkstarttime + 599)
        connect_nodes(self.nodes[0],3)
        sync_blocks([self.nodes[0], self.nodes[3]])
        # The fork is penalized, but not enough to reduce it to node0's chainwork
        # node0 switches to node3's chain
        assert_equal(self.nodes[0].getbestblockhash(), node3tiphash)
        assert_equal(self.nodes[3].getbestblockhash(), node3tiphash)

if __name__ == '__main__':
    ReorgResistanceTest ().main ()
