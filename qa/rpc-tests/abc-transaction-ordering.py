#!/usr/bin/env python3
# Copyright (c) 2015-2016 The Bitcoin Core developers
# Copyright (c) 2017 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
This test checks that the nod esoftware accepts transactions in
non topological order once the feature is activated.
"""

from test_framework.test_framework import ComparisonTestFramework
from test_framework.util import assert_equal, assert_raises_jsonrpc, start_nodes
from test_framework.comptool import TestManager, TestInstance, RejectResult
from test_framework.blocktools import *
import time
from test_framework.key import CECKey
from test_framework.script import *
from collections import deque

# far into the future
AOR_ACTIVATION_TIME = 2000000000


class PreviousSpendableOutput():

    def __init__(self, tx=CTransaction(), n=-1):
        self.tx = tx
        self.n = n  # the output we're spending


class TransactionOrderingTest(ComparisonTestFramework):

    def __init__(self):
        super().__init__()
        self.set_test_params()

    # Can either run this test as 1 node with expected answers, or two and compare them.
    # Change the "outcome" variable from each TestInstance object to only do
    # the comparison.

    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
        self.block_heights = {}
        self.tip = None
        self.blocks = {}
        self.extra_args = [['-whitelist=127.0.0.1',
                            "-fourthhftime=%d" % AOR_ACTIVATION_TIME ]]
        self.dbg = self.log.debug

    def setup_network(self):
        self.nodes = start_nodes(
            self.num_nodes, self.options.tmpdir,
            extra_args=self.extra_args * self.num_nodes,
            binary=[self.options.testbinary] +
            [self.options.refbinary]*(self.num_nodes-1))

    def run_test(self):
        self.test = TestManager(self, self.options.tmpdir)
        self.test.add_all_connections(self.nodes)
        NetworkThread().start()

        # knock the node out of IBD
        self.nodes[0].generate(1)

        self.nodes[0].setmocktime(AOR_ACTIVATION_TIME)
        self.test.run()

    def add_transactions_to_block(self, block, tx_list):
        [tx.rehash() for tx in tx_list]
        block.vtx.extend(tx_list)

    # this is a little handier to use than the version in blocktools.py
    def create_tx(self, spend, value, script=CScript([OP_TRUE])):
        tx = create_transaction(spend.tx, spend.n, b"", value, script)
        return tx

    def next_block(self, number, spend=None, tx_count=0):
        if self.tip == None:
            base_block_hash = self.genesis_hash
            block_time = int(time.time()) + 1
        else:
            base_block_hash = self.tip.sha256
            block_time = self.tip.nTime + 1
        # First create the coinbase
        height = self.block_heights[base_block_hash] + 1
        coinbase = create_coinbase(height)
        coinbase.rehash()
        if spend == None:
            # We need to have something to spend to fill the block.
            block = create_block(base_block_hash, coinbase, block_time)
        else:
            # all but one satoshi to fees
            coinbase.vout[0].nValue += spend.tx.vout[spend.n].nValue - 1
            coinbase.rehash()
            block = create_block(base_block_hash, coinbase, block_time)

            # Make sure we have plenty enough to spend going forward.
            spendable_outputs = deque([spend])

            def get_base_transaction():
                # Create the new transaction
                tx = CTransaction()
                # Spend from one of the spendable outputs
                spend = spendable_outputs.popleft()
                tx.vin.append(CTxIn(COutPoint(spend.tx.sha256, spend.n)))
                # Add spendable outputs
                for i in range(4):
                    tx.vout.append(CTxOut(0, CScript([OP_TRUE])))
                    spendable_outputs.append(PreviousSpendableOutput(tx, i))
                return tx

            tx = get_base_transaction()

            # Make it the same format as transaction added for padding and save the size.
            # It's missing the padding output, so we add a constant to account for it.
            tx.rehash()
            base_tx_size = len(tx.serialize()) + 18

            # Put some random data into the first transaction of the chain to randomize ids.
            tx.vout.append(
                CTxOut(0, CScript([random.randint(0, 256), OP_RETURN])))

            # Add the transaction to the block
            self.add_transactions_to_block(block, [tx])

            # If we have a transaction count requirement, just fill the block until we get there
            while len(block.vtx) < tx_count:
                # Create the new transaction and add it.
                tx = get_base_transaction()
                self.add_transactions_to_block(block, [tx])

            # Now that we added a bunch of transaction, we need to recompute
            # the merkle root.
            block.hashMerkleRoot = block.calc_merkle_root()

        if tx_count > 0:
            assert_equal(len(block.vtx), tx_count)

        # Do PoW, which is cheap on regnet
        block.solve()
        self.tip = block
        self.block_heights[block.sha256] = height
        assert number not in self.blocks
        self.blocks[number] = block
        return block

    def get_tests(self):
        node = self.nodes[0]
        self.genesis_hash = int(node.getbestblockhash(), 16)
        self.block_heights[self.genesis_hash] = 0
        spendable_outputs = []

        # save the current tip so it can be spent by a later block
        def save_spendable_output():
            spendable_outputs.append(self.tip)

        # get an output that we previously marked as spendable
        def get_spendable_output():
            return PreviousSpendableOutput(spendable_outputs.pop(0).vtx[0], 0)

        # returns a test case that asserts that the current tip was accepted
        def accepted():
            return TestInstance([[self.tip, True]])

        # returns a test case that asserts that the current tip was rejected
        def rejected(reject=None):
            if reject is None:
                return TestInstance([[self.tip, False]])
            else:
                return TestInstance([[self.tip, reject]])

        # move the tip back to a previous block
        def tip(number):
            self.tip = self.blocks[number]

        # adds transactions to the block and updates state
        def update_block(block_number, new_transactions=[]):
            block = self.blocks[block_number]
            self.add_transactions_to_block(block, new_transactions)
            old_sha256 = block.sha256
            block.hashMerkleRoot = block.calc_merkle_root()
            block.solve()
            # Update the internal state just like in next_block
            self.tip = block
            if block.sha256 != old_sha256:
                self.block_heights[block.sha256] = self.block_heights[old_sha256]
                del self.block_heights[old_sha256]
            self.blocks[block_number] = block
            return block

        # shorthand for functions
        block = self.next_block

        self.dbg("Create a new block")
        block(0)
        save_spendable_output()
        yield accepted()

        self.dbg("Now we need that block to mature so we can spend the coinbase.")
        test = TestInstance(sync_every_block=False)
        for i in range(99):
            block(5000 + i)
            test.blocks_and_transactions.append([self.tip, True])
            save_spendable_output()
        yield test

        # collect spendable outputs now to avoid cluttering the code later on
        out = []
        for i in range(100):
            out.append(get_spendable_output())

        self.dbg("Let's build some blocks and test them.")
        for i in range(15):
            n = i + 1
            block(n)
            yield accepted()

        self.dbg("Start moving MTP forward")
        bfork = block(5555)
        bfork.nTime = AOR_ACTIVATION_TIME - 1
        update_block(5555)
        yield accepted()

        self.dbg("Get to one block of the Nov 15, 2018 HF activation")
        for i in range(5):
            block(5100 + i)
            test.blocks_and_transactions.append([self.tip, True])
        yield test

        self.dbg("Check that the MTP is just before the configured fork point.")
        assert_equal(node.getblockheader(node.getbestblockhash())['mediantime'],
                     AOR_ACTIVATION_TIME - 1)

        self.dbg("Before we activate the Nov 15, 2018 HF, transaction order is respected.")
        def out_of_order_block(block_number, spend):
            b = block(block_number, spend=spend, tx_count=3)
            b.vtx[1], b.vtx[2] = b.vtx[2], b.vtx[1]
            update_block(block_number)
            return b

        out_of_order_block(4444, out[16])
        yield rejected(RejectResult(16, b'bad-txns-inputs-missingorspent'))

        # Rewind bad block.
        tip(5104)

        self.dbg("Activate the Nov 15, 2018 HF")
        block(5556)
        yield accepted()

        self.dbg("Now MTP is exactly the fork time.")
        assert_equal(node.getblockheader(node.getbestblockhash())['mediantime'],
                     AOR_ACTIVATION_TIME)

        self.dbg("Now that the fork activated, we can put transactions out of order in the block.")
        out_of_order_block(4445, out[16])
        yield accepted()

        oooblockhash = node.getbestblockhash()
        node.invalidateblock(oooblockhash)
        assert(node.getbestblockhash() != oooblockhash)


if __name__ == '__main__':
    TransactionOrderingTest().main()
