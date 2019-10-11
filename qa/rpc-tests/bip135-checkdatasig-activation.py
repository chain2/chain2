#!/usr/bin/env python3
# Copyright (c) 2019 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
This test checks BIP135 activation of OP_CHECKDATASIG
"""

from test_framework.test_framework import ComparisonTestFramework
from test_framework.util import *
from test_framework.comptool import TestManager, TestInstance, RejectResult
from test_framework.blocktools import *
from test_framework.script import *

# Error due to invalid opcodes
BAD_OPCODE_ERROR = b'mandatory-script-verify-flag-failed (Opcode missing or not understood)'
RPC_BAD_OPCODE_ERROR = "16: " + \
    BAD_OPCODE_ERROR.decode("utf-8")


class PreviousSpendableOutput():

    def __init__(self, tx=CTransaction(), n=-1):
        self.tx = tx
        self.n = n  # the output we're spending


class CheckDataSigActivationTest(ComparisonTestFramework):

    def __init__(self):
        super().__init__()
        self.set_test_params()

    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
        self.extra_args = [['-whitelist=127.0.0.1']]

    def setup_network(self):
        # blocks are 1 second apart by default in this regtest
        # regtest bip135 deployments are defined for a blockchain that starts at MOCKTIME
        enable_mocktime()
        self.nodes = start_nodes(
                self.num_nodes, self.options.tmpdir,
                extra_args=self.extra_args * self.num_nodes,
                binary=[self.options.testbinary] +
                [self.options.refbinary]*(self.num_nodes-1))

    def create_checkdatasig_tx(self, count):
        node = self.nodes[0]
        utxos = node.listunspent()
        assert(len(utxos) > 0)
        utxo = utxos[0]
        tx = CTransaction()
        value = int(satoshi_round(utxo["amount"]) * COIN) // count
        tx.vin = [CTxIn(COutPoint(int(utxo["txid"], 16), utxo["vout"]))]
        tx.vout = []
        signature = bytearray.fromhex(
            '30440220256c12175e809381f97637933ed6ab97737d263eaaebca6add21bced67fd12a402205ce29ecc1369d6fc1b51977ed38faaf41119e3be1d7edfafd7cfaf0b6061bd07')
        message = bytearray.fromhex('')
        pubkey = bytearray.fromhex(
            '038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508')
        for _ in range(count):
            tx.vout.append(CTxOut(value, CScript(
                [signature, message, pubkey, OP_CHECKDATASIG])))
        tx.vout[0].nValue -= get_relay_fee(node, unit = "sat")
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

        # First, we generate some coins to spend.
        node.generate(125)

        # Create various outputs using the OP_CHECKDATASIG
        # to check for activation.
        tx_hex = self.create_checkdatasig_tx(25)
        txid = node.sendrawtransaction(tx_hex)
        assert(txid in set(node.getrawmempool()))

        node.generate(1)
        assert(txid not in set(node.getrawmempool()))

        # register the spendable outputs.
        tx = FromHex(CTransaction(), tx_hex)
        tx.rehash()
        spendable_checkdatasigs = [PreviousSpendableOutput(tx, i)
                                   for i in range(len(tx.vout))]

        def spend_checkdatasig():
            outpoint = spendable_checkdatasigs.pop()
            out = outpoint.tx.vout[outpoint.n]
            tx = CTransaction()
            tx.vin = [CTxIn(COutPoint(outpoint.tx.sha256, outpoint.n))]
            tx.vout = [CTxOut(out.nValue, CScript([])),
                       CTxOut(0, CScript([random.getrandbits(800), OP_RETURN]))]
            tx.vout[0].nValue -= get_relay_fee(node, unit = "sat")
            tx.rehash()
            return tx

        # Check that transactions using checkdatasig are not accepted yet.
        self.log.info("Try to use the checkdatasig opcodes before activation")

        tx0 = spend_checkdatasig()
        tx0_hex = ToHex(tx0)
        assert_raises_rpc_error(-26, RPC_BAD_OPCODE_ERROR,
                                node.sendrawtransaction, tx0_hex)
        assert_equal(get_bip135_status(node, 'bip135test0')['status'], 'started')

        # CDSV regtest start happened at height 99, activation should be at height 299
        self.log.info("Advance to height 298, just before activation, and check again")
        node.generate(172)

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
            tiphash = node.getbestblockhash()
            tipheader = node.getblockheader(tiphash)
            height = int(tipheader['height'])

            # create the block
            coinbase = create_coinbase(absoluteHeight=height+1)
            coinbase.rehash()
            version = 0x20000001 # Signal for CDSV on bit 0
            block = create_block(int(tiphash, 16), coinbase,
                                 int(tipheader['time']) + 600, nVersion=version)

            # Do PoW, which is cheap on regnet
            block.solve()
            return block

        assert_raises_rpc_error(-26, RPC_BAD_OPCODE_ERROR,
                                node.sendrawtransaction, tx0_hex)

        def add_tx(block, tx):
            block.vtx.append(tx)
            block.hashMerkleRoot = block.calc_merkle_root()
            block.solve()

        b = next_block()
        add_tx(b, tx0)
        yield rejected(b, RejectResult(16, b'blk-bad-inputs')) #1

        assert_equal(get_bip135_status(node, 'bip135test0')['status'], 'locked_in')


        self.log.info("Activates checkdatasig")
        fork_block = next_block()
        yield accepted(fork_block) #2

        assert_equal(get_bip135_status(node, 'bip135test0')['status'], 'active')

        tx0id = node.sendrawtransaction(tx0_hex)
        assert(tx0id in set(node.getrawmempool()))

        # Transactions can also be included in blocks.
        nextblock = next_block()
        add_tx(nextblock, tx0)
        yield accepted(nextblock) #3

        self.log.info("Cause a reorg that deactivate the checkdatasig opcodes")

        # Invalidate the checkdatasig block, ensure tx0 gets back to the mempool.
        assert(tx0id not in set(node.getrawmempool()))

        node.invalidateblock(format(nextblock.sha256, 'x'))
        assert(tx0id in set(node.getrawmempool()))

        node.invalidateblock(format(fork_block.sha256, 'x'))
        assert(tx0id not in set(node.getrawmempool()))


if __name__ == '__main__':
    CheckDataSigActivationTest().main()
