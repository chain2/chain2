#!/usr/bin/env python3
# Copyright (c) 2017 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test mining and broadcast of larger-than-1MB-blocks
#
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from test_framework.mininode import MAX_BLOCK_SIZE

from decimal import Decimal
CACHE_DIR = "cache_bigblock"

class BigBlockTest(BitcoinTestFramework):

    def __init__(self):
        super().__init__()
        self.num_nodes = 4
        self.setup_clean_chain = None
        # Use predictable timestamps on subsequent runs from cache
        enable_mocktime()

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)

        if not os.path.isdir(os.path.join(CACHE_DIR, "node0")):
            print("Creating initial chain. This will be cached for future runs.")

            for i in range(4):
                initialize_datadir(CACHE_DIR, i) # Overwrite port/rpcport in bitcoin.conf

            # Node 0 creates 10MB blocks that vote for increase to 10MB
            # Node 1 creates empty blocks that vote for 10MB
            # Node 2 creates empty blocks that do not vote for increase
            # Node 3 creates empty blocks that vote for 9MB
            self.nodes = []
            # Use node0 to mine blocks for input splitting
            self.nodes.append(start_node(0, CACHE_DIR, ["-blockmaxsize=35000000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=300))
            self.nodes.append(start_node(1, CACHE_DIR, ["-blockmaxsize=1000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=300))
            self.nodes.append(start_node(2, CACHE_DIR, ["-blockmaxsize=1000", "-maxblocksizevote=32", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=300))
            self.nodes.append(start_node(3, CACHE_DIR, ["-blockmaxsize=99999", "-maxblocksizevote=34", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=300))

            connect_nodes_bi(self.nodes, 0, 1)
            connect_nodes_bi(self.nodes, 0, 2)
            connect_nodes_bi(self.nodes, 0, 3)
            connect_nodes_bi(self.nodes, 1, 2)
            connect_nodes_bi(self.nodes, 1, 3)
            connect_nodes_bi(self.nodes, 2, 3)

            self.is_network_split = False

            # Create a 2012-block chain in a 75% ratio for increase (genesis block votes for 32MB)
            # Make sure they are not already sorted correctly
            self.nodes[1].generate(503)
            assert(sync_blocks(self.nodes[1:3], timeout=120))
            self.nodes[2].generate(502) # <--- genesis is 503rd vote for no increase
            assert(sync_blocks(self.nodes[2:4], timeout=120))

            relayfee = self.nodes[3].getnetworkinfo()['relayfee']
            utxos = create_confirmed_utxos(relayfee, self.nodes[3], 512)

            # 358 node 3 blocks were already mined to create utxos
            self.nodes[3].generate(503 - 358)
            assert(sync_blocks(self.nodes[1:4], timeout=120))
            self.nodes[1].generate(503)
            assert(sync_blocks(self.nodes, timeout=120))

            print("Creating transaction data")
            txouts = gen_return_txouts()
            tx_file = open(os.path.join(CACHE_DIR, "txdata"), "w")
            create_lots_of_big_transactions(self.nodes[3], txouts, utxos, 512, relayfee * 1000, tx_file)
            tx_file.close()

            stop_nodes(self.nodes)
            self.nodes = []
            for i in range(4):
                os.remove(log_filename(CACHE_DIR, i, "db.log"))
                os.remove(log_filename(CACHE_DIR, i, "peers.dat"))
                os.remove(log_filename(CACHE_DIR, i, "fee_estimates.dat"))

        for i in range(4):
            from_dir = os.path.join(CACHE_DIR, "node"+str(i))
            to_dir = os.path.join(self.options.tmpdir,  "node"+str(i))
            shutil.copytree(from_dir, to_dir)
            initialize_datadir(self.options.tmpdir, i) # Overwrite port/rpcport in bitcoin.conf

    def setup_network(self):
        self.nodes = []

        self.nodes.append(start_node(0, self.options.tmpdir, ["-blockmaxsize=35000000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=60))
        self.nodes.append(start_node(1, self.options.tmpdir, ["-blockmaxsize=1000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=60))
        self.nodes.append(start_node(2, self.options.tmpdir, ["-blockmaxsize=1000", "-maxblocksizevote=32", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=60))
        # (We don't restart the node with the huge wallet
        connect_nodes_bi(self.nodes, 0, 1)
        connect_nodes_bi(self.nodes, 1, 2)
        connect_nodes_bi(self.nodes, 2, 0)

        self.load_mempool(self.nodes[0])

    def load_mempool(self, node):
        with open(os.path.join(CACHE_DIR, "txdata"), "r") as f:
            for line in f:
                node.sendrawtransaction(line.rstrip())

    def TestMineBig(self, expect_big):
        # Test if node0 will mine a block bigger than MAX_BLOCK_SIZE
        b1hash = self.nodes[0].generate(1)[0]
        b1 = self.nodes[0].getblock(b1hash, True)
        assert(sync_blocks(self.nodes[0:3]))

        if expect_big:
            assert(b1['size'] > MAX_BLOCK_SIZE)

            # Have node1 mine on top of the block,
            # to make sure it goes along with the fork
            b2hash = self.nodes[1].generate(1)[0]
            b2 = self.nodes[1].getblock(b2hash, True)
            assert(b2['previousblockhash'] == b1hash)
            assert(sync_blocks(self.nodes[0:3]))

        else:
            assert(b1['size'] <= MAX_BLOCK_SIZE)

        # Reset chain to before b1hash:
        for node in self.nodes[0:3]:
            node.invalidateblock(b1hash)
        assert(sync_blocks(self.nodes[0:3]))


    def run_test(self):
        print("Testing consensus blocksize increase conditions")

        gbtRequest = {'rules':['testdummy']}

        assert_equal(self.nodes[0].getblockcount(), 2011) # This is a 0-based height

        # Current nMaxBlockSize is still 8MB
        assert_equal(self.nodes[0].getblocktemplate(gbtRequest)["sizelimit"], MAX_BLOCK_SIZE)
        self.TestMineBig(False)

        # Create a situation where the 1512th-highest vote is for 34MB
        self.nodes[2].generate(1)
        assert(sync_blocks(self.nodes[1:3]))
        ahash = self.nodes[1].generate(3)[2]
        assert_equal(self.nodes[1].getblocktemplate(gbtRequest)["sizelimit"], int(MAX_BLOCK_SIZE * 1.05))
        assert(sync_blocks(self.nodes[0:2]))
        self.TestMineBig(True)

        # Shutdown then restart node[0], it should produce a big block.
        stop_node(self.nodes[0], 0)
        self.nodes[0] = start_node(0, self.options.tmpdir, ["-blockmaxsize=35000000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=60)
        self.load_mempool(self.nodes[0])
        connect_nodes_bi(self.nodes, 0, 1)
        connect_nodes_bi(self.nodes, 0, 2)
        assert_equal(self.nodes[0].getblocktemplate(gbtRequest)["sizelimit"], int(MAX_BLOCK_SIZE * 1.05))
        self.TestMineBig(True)

        # Test re-orgs past the sizechange block
        stop_node(self.nodes[0], 0)
        self.nodes[2].invalidateblock(ahash)
        assert_equal(self.nodes[2].getblocktemplate(gbtRequest)["sizelimit"], MAX_BLOCK_SIZE)
        self.nodes[2].generate(2)
        assert_equal(self.nodes[2].getblocktemplate(gbtRequest)["sizelimit"], MAX_BLOCK_SIZE)
        assert(sync_blocks(self.nodes[1:3]))

        # Restart node0, it should re-org onto longer chain,
        # and refuse to mine a big block:
        self.nodes[0] = start_node(0, self.options.tmpdir, ["-blockmaxsize=35000000", "-maxblocksizevote=35", "-limitancestorsize=2000", "-limitdescendantsize=2000"], timewait=60)
        self.load_mempool(self.nodes[0])
        connect_nodes_bi(self.nodes, 0, 1)
        connect_nodes_bi(self.nodes, 0, 2)
        assert(sync_blocks(self.nodes[0:3]))
        assert_equal(self.nodes[0].getblocktemplate(gbtRequest)["sizelimit"], MAX_BLOCK_SIZE)
        self.TestMineBig(False)

        # Mine 4 blocks voting for 35MB. Bigger block NOT ok, we are in the next voting period
        self.nodes[1].generate(4)
        assert_equal(self.nodes[1].getblocktemplate(gbtRequest)["sizelimit"], MAX_BLOCK_SIZE)
        assert(sync_blocks(self.nodes[0:3]))
        self.TestMineBig(False)


        print("Cached test chain and transactions left in %s"%(CACHE_DIR))

if __name__ == '__main__':
    BigBlockTest().main()
