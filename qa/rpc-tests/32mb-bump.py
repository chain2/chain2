#!/usr/bin/env python3
# Copyright (c) 2018 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from test_framework.mininode import ONE_MEGABYTE

"""
Originally a test of 32MB max block size bump, kept as test of 32MB submitblock
"""

MB = ONE_MEGABYTE

# Limit of previous block mined
def get_sizelimit(node):
    return node.getmininginfo()['sizelimit']

def assert_chaintip_within(node, lower, upper):
        block = node.getblock(node.getbestblockhash(), True)
        assert(block['size'] > int(lower) and block['size'] < int(upper))

"""
Mine a block close to target_size, first mining blocks to create utxos if needed.
"""
def mine_block(node, target_bytes, utxos):
    # we will make create_lots_of_big_transactions create a bunch of ~67.5k tx
    import math
    required_txs = math.floor(target_bytes / 67500)

    # if needed, create enough utxos to be able to mine target_size
    fee = 100 * node.getnetworkinfo()["relayfee"]
    if len(utxos) < required_txs:
        utxos.clear()
        utxos.extend(create_confirmed_utxos(fee, node, required_txs))

    create_lots_of_big_transactions(node, gen_return_txouts(), utxos, required_txs, fee)
    node.generate(1)

class HFBumpTest(BitcoinTestFramework):

    def __init__(self):
        super().__init__()
        self.mocktime = 0

    def generate_one(self):
        assert(self.mocktime > 0)
        self.mocktime = self.mocktime + 600
        set_node_times(self.nodes, self.mocktime)
        self.nodes[0].generate(1)

    def run_test(self):
        self._test_mine_big_block()
        self._test_submit_big_block()

    def _test_mine_big_block(self):
        print("Test that we can mine a 32MB block")
        utxo_cache = []
        node = self.nodes[0]
        fee = 100  * node.getnetworkinfo()["relayfee"]

        # Mine a block close to 32MB
        target = 32 * MB
        mine_block(node, target_bytes = target, utxos = utxo_cache)
        assert_chaintip_within(node, target - 0.1*MB, target)

        # Clear the mempool
        while node.getmempoolinfo()['bytes']:
            node.generate(1)

        # All nodes should agree on best block
        sync_blocks(self.nodes)
        print("OK!")

    def _test_submit_big_block(self):
        print("Test that we can submit big block with RPC interface")
        # Mine a big block with node 0, then confirm that we can manually submit it to node 1.

        stop_node(self.nodes[1], 1) # To make sure it disconnects from node 0
        start_node(1, self.options.tmpdir, ["-debug", "-mocktime=%s" % self.mocktime])

        utxo_cache = [ ]
        too_big_target = 42 * MB
        mine_block(self.nodes[0], target_bytes = too_big_target, utxos = utxo_cache)
        assert_chaintip_within(self.nodes[0], 31.9*MB, 32*MB)

        assert(self.nodes[0].getbestblockhash() != self.nodes[1].getbestblockhash())

        while True:
            node0_tip = self.nodes[0].getblockcount()
            node1_tip = self.nodes[1].getblockcount()
            if node1_tip >= node0_tip:
                break
            block_hash = self.nodes[0].getblockhash(node1_tip + 1)
            self.nodes[1].submitblock(self.nodes[0].getblock(block_hash, False))

        assert(self.nodes[0].getbestblockhash() == self.nodes[1].getbestblockhash())

if __name__ == '__main__':
    HFBumpTest().main()
