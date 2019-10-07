#!/usr/bin/env python3
# Copyright (c) 2015-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import ComparisonTestFramework
from test_framework.util import *
from test_framework.mininode import CTransaction, NetworkThread
from test_framework.blocktools import create_coinbase, create_block
from test_framework.comptool import TestInstance, TestManager
from test_framework.script import *
from io import BytesIO
import time

def cltv_invalidate(tx):
    '''Modify the signature in vin 0 of the tx to fail CLTV

    Prepends -1 CLTV DROP in the scriptSig itself.
    '''
    tx.vin[0].scriptSig = CScript([OP_1NEGATE, OP_CHECKLOCKTIMEVERIFY, OP_DROP] +
                                  list(CScript(tx.vin[0].scriptSig)))

def cltv_exercise(tx):
    '''Modify the signature in vin 0 of the tx to exercise CLTV

    Prepends 0 CLTV DROP in the scriptSig itself.
    '''
    tx.vin[0].scriptSig = CScript([OP_0, OP_CHECKLOCKTIMEVERIFY, OP_DROP] +
                                  list(CScript(tx.vin[0].scriptSig)))

'''
This test is meant to exercise BIP65 (CHECKLOCKTIMEVERIFY)
'''

class BIP65Test(ComparisonTestFramework):

    def __init__(self):
        super().__init__()
        self.num_nodes = 1

    def setup_network(self):
        # Must set the blockversion for this test
        self.nodes = start_nodes(self.num_nodes, self.options.tmpdir,
                                 extra_args=[['-whitelist=127.0.0.1']],
                                 binary=[self.options.testbinary])

    def run_test(self):
        test = TestManager(self, self.options.tmpdir)
        test.add_all_connections(self.nodes)
        NetworkThread().start() # Start up network handling in another thread
        test.run()

    def create_transaction(self, node, coinbase, to_address, amount):
        from_txid = node.getblock(coinbase)['tx'][0]
        inputs = [{ "txid" : from_txid, "vout" : 0, "sequence" : 4294967294}]
        outputs = { to_address : amount }
        rawtx = node.createrawtransaction(inputs, outputs)
        signresult = node.signrawtransaction(rawtx)
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(signresult['hex']))
        tx.deserialize(f)
        return tx

    def get_tests(self):
        self.coinbase_blocks = self.nodes[0].generate(2)
        self.nodes[0].generate(100)
        height = 103 # height of the next block to build
        self.tip = int ("0x" + self.nodes[0].getbestblockhash(), 0)
        self.nodeaddress = self.nodes[0].getnewaddress()
        self.last_block_time = int(time.time() + 1000)       

        self.log.info("Check that CLTV rules are enforced")
        
        self.log.info("Attempt to mine a block containing invalid use of CLTV")        
        spendtx = self.create_transaction(self.nodes[0], self.coinbase_blocks[1], self.nodeaddress, 1.0)
        cltv_invalidate(spendtx)
        spendtx.rehash()
        block = create_block(self.tip, create_coinbase(absoluteHeight = height), self.last_block_time + 1)
        block.vtx.append(spendtx)
        block.hashMerkleRoot = block.calc_merkle_root()
        block.rehash()
        block.solve()
        yield TestInstance([[block, False]])

        self.log.info("Attempt to mine an old version block, should be invalid")
        block = create_block(self.tip, create_coinbase(absoluteHeight = height), self.last_block_time + 1)
        block.nVersion = 3
        block.rehash()
        block.solve()
        yield TestInstance([[block, False]])

        self.log.info("Mine a block that exercises CLTV")
        spendtx = self.create_transaction(self.nodes[0], self.coinbase_blocks[1], self.nodeaddress, 1.0)
        cltv_exercise(spendtx)
        spendtx.rehash()
        block = create_block(self.tip, create_coinbase(absoluteHeight = height), self.last_block_time + 1)
        block.vtx.append(spendtx)
        block.hashMerkleRoot = block.calc_merkle_root()
        block.rehash()
        block.solve()
        yield TestInstance([[block, True]])

if __name__ == '__main__':
    BIP65Test().main()
