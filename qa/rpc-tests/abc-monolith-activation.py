#!/usr/bin/env python3
# Copyright (c) 2018 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
Originally a test of monolith activation, kept as a p2p test of OP_AND validity 
"""

from test_framework.test_framework import ComparisonTestFramework
from test_framework.util import satoshi_round, assert_equal, assert_raises_jsonrpc, start_nodes
from test_framework.comptool import TestManager, TestInstance, RejectResult
from test_framework.blocktools import *
from test_framework.script import *

class PreviousSpendableOutput():

    def __init__(self, tx=CTransaction(), n=-1):
        self.tx = tx
        self.n = n  # the output we're spending


class MonolithActivationTest(ComparisonTestFramework):

    def setup_network(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
        self.extra_args = [['-whitelist=127.0.0.1']]

        self.nodes = start_nodes(
            self.num_nodes, self.options.tmpdir,
            extra_args=self.extra_args * self.num_nodes,
            binary=[self.options.testbinary] +
            [self.options.refbinary]*(self.num_nodes-1))

    def create_and_tx(self, count):
        node = self.nodes[0]
        utxos = node.listunspent()
        assert(len(utxos) > 0)
        utxo = utxos[0]
        tx = CTransaction()
        value = int(satoshi_round(
            utxo["amount"] - self.relayfee) * COIN) // count
        tx.vin = [CTxIn(COutPoint(int(utxo["txid"], 16), utxo["vout"]))]
        tx.vout = []
        for _ in range(count):
            tx.vout.append(CTxOut(value, CScript([OP_1, OP_1, OP_AND])))
        tx_signed = node.signrawtransaction(ToHex(tx))["hex"]
        return tx_signed

    def run_test(self):
        self.test = TestManager(self, self.options.tmpdir)
        self.test.add_all_connections(self.nodes)
        # Start up network handling in another thread
        NetworkThread().start()
        self.test.run()

    def get_tests(self):
        node = self.nodes[0]
        self.relayfee = self.nodes[0].getnetworkinfo()["relayfee"]

        # First, we generate some coins to spend.
        node.generate(125)

        # Create various outputs using the OP_AND to check for activation.
        tx_hex = self.create_and_tx(25)
        txid = node.sendrawtransaction(tx_hex)
        assert(txid in set(node.getrawmempool()))

        node.generate(1)
        assert(txid not in set(node.getrawmempool()))

        # register the spendable outputs.
        tx = FromHex(CTransaction(), tx_hex)
        tx.rehash()
        spendable_ands = [PreviousSpendableOutput(
            tx, i) for i in range(len(tx.vout))]

        def spend_and():
            outpoint = spendable_ands.pop()
            out = outpoint.tx.vout[outpoint.n]
            value = int(out.nValue - (self.relayfee * COIN))
            tx = CTransaction()
            tx.vin = [CTxIn(COutPoint(outpoint.tx.sha256, outpoint.n))]
            tx.vout = [CTxOut(value, CScript([]))]
            tx.rehash()
            return tx

        tx0 = spend_and()
        tx0_hex = ToHex(tx0)

        # returns a test case that asserts that the current tip was accepted
        def accepted(tip):
            return TestInstance([[tip, True]])

        # returns a test case that asserts that the current tip was rejected
        def rejected(tip, reject=None):
            if reject is None:
                return TestInstance([[tip, False]])
            else:
                return TestInstance([[tip, reject]])

        def next_block():
            # get block height
            blockchaininfo = node.getblockchaininfo()
            height = int(blockchaininfo['blocks']) + 1
            prevtime = node.getblockheader(blockchaininfo['bestblockhash'])['time']

            # create the block
            coinbase = create_coinbase(absoluteHeight = height)
            coinbase.rehash()
            block = create_block(
                int(node.getbestblockhash(), 16), coinbase, prevtime + 600)

            # Do PoW, which is cheap on regnet
            block.solve()
            return block

        def add_tx(block, tx):
            block.vtx.append(tx)
            block.hashMerkleRoot = block.calc_merkle_root()
            block.solve()

        b = next_block()
        add_tx(b, tx0)

        tx0id = node.sendrawtransaction(tx0_hex)
        assert(tx0id in set(node.getrawmempool()))

        # Transactions can also be included in blocks.
        monolithblock = next_block()
        add_tx(monolithblock, tx0)
        yield accepted(monolithblock)

if __name__ == '__main__':
    MonolithActivationTest().main()
