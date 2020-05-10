// Copyright (c) 2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "main.h"
#include "test/test_bitcoin.h"
#include "consensus/consensus.h" // for MAX_BLOCK_SIZE

#include <boost/test/unit_test.hpp>
#include <set>


extern std::set<CBlockIndex*, CBlockIndexWorkComparator> setBlockIndexCandidates;
extern CBlockIndex* FindMostWorkChain();

BOOST_FIXTURE_TEST_SUITE(penalty_tests, TestChain110Setup)

CBlockIndex AddBlock(CBlockIndex& cousin, int blocksBack, unsigned int nBits, uint32_t nSequenceId, int64_t nTimeDataReceived)
{
    BOOST_ASSERT(blocksBack <= 0);

    CBlockIndex *parent = cousin.GetAncestor(cousin.nHeight + blocksBack);
    CBlockIndex newBlock;
    newBlock.nStatus = BLOCK_HAVE_DATA;
    newBlock.nChainTx = 1;
    newBlock.pprev = parent;
    newBlock.nHeight = parent ? parent->nHeight + 1 : 1;
    newBlock.nBits = nBits;
    arith_uint256 work = BitsToWork(nBits);
    newBlock.nChainWork = parent ? parent->nChainWork + work : work;
    newBlock.nSequenceId = nSequenceId;
    newBlock.nTimeDataReceived = nTimeDataReceived;
    newBlock.nTime = nTimeDataReceived;

    return newBlock;
}

BOOST_AUTO_TEST_CASE(late_fork_penalty) {
{
    LOCK(cs_main);

    // Verify current tip is most-work
    CBlockIndex *tip = chainActive.Tip();
    BOOST_CHECK_EQUAL(FindMostWorkChain(), tip);

    uint32_t nSequenceId = tip->nSequenceId;

    // fork1: PCW = tipPCW, penalty = 0 bits
    // 1-block fork with same parent as tip has equal parent chainwork, regardless of its own higher work
    // tip-2 <- tip-1 (0x207fffff) <- tip (0x207fffff)
    //              \              <- fork1_1 (0x201fffff, +0)
    CBlockIndex fork1_1 = AddBlock(*tip, -1, 0x201fffff, ++nSequenceId, tip->nTime);
    setBlockIndexCandidates.insert(&fork1_1);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), tip);

    // fork2: PCW = tipPCW - 2 + (4 @ penalty 0) = tipPCW + 2
    // 2-block fork with more PCW than tip, received barely without penalty
    // tip-2 <- tip-1 (0x207fffff)                <- tip (0x207fffff)
    //   |          \                             <- fork1_1 (0x201fffff, +0)
    //   \   <- fork2_1 (0x203fffff, tip-1 + 599)  <- fork2_2 (0x207fffff, +1)
    CBlockIndex fork2_1 = AddBlock(*tip, -2, 0x203fffff, ++nSequenceId, tip->pprev->nTime + 599);
    CBlockIndex fork2_2 = AddBlock(fork2_1, 0, 0x207fffff, ++nSequenceId, fork2_1.nTime + 1);
    setBlockIndexCandidates.insert(&fork2_2);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), &fork2_2);

    // fork3: PCW = tipPCW - 2 + (6 @ penalty 2^2) = tipPCW - 1
    // 2-block fork whose first block has more work than fork2_1, but penalty reduces it below tipPCW
    // tip-2 <- tip-1 (0x207fffff)                 <- tip (0x207fffff)
    //   |          \                              <- fork1_1 (0x201fffff, +0)
    //   \   <- fork2_1 (0x203fffff, tip-1 + 599)  <- fork2_2 (0x207fffff, +1)
    //   \   <- fork3_1 (0x2027ffff, tip-1 + 1800) <- fork3_2 (0x200fffff, +5400)
    CBlockIndex fork3_1 = AddBlock(*tip, -2, 0x2027ffff, ++nSequenceId, tip->pprev->nTime + 1800);
    CBlockIndex fork3_2 = AddBlock(fork3_1, 0, 0x200fffff, ++nSequenceId, fork3_1.nTime + 5400);
    setBlockIndexCandidates.insert(&fork3_2);
    BOOST_CHECK_EQUAL(GetPenalizedWork(fork3_1, tip->pprev->nTime).GetLow64(), 1);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), &fork2_2);

    // fork4: PCW = tipPCW - 2 + (6 @ penalty 2^1) = tipPCW + 1
    // 2-block fork whose first block has more work than fork2_1, not enough penalty to reduce it below tipPCW
    // tip-2 <- tip-1 (0x207fffff)                 <- tip (0x207fffff)
    //   |          \                              <- fork1_1 (0x201fffff, +0)
    //   \   <- fork2_1 (0x203fffff, tip-1 + 599)  <- fork2_2 (0x207fffff, +1)
    //   \   <- fork3_1 (0x2027ffff, tip-1 + 1800) <- fork3_2 (0x200fffff, +5400)
    //   \   <- fork4_1 (0x2027ffff, tip-1 + 600)  <- fork4_2 (0x207fffff, +1)
    CBlockIndex fork4_1 = AddBlock(*tip, -2, 0x2027ffff, ++nSequenceId, tip->pprev->nTime + 600);
    CBlockIndex fork4_2 = AddBlock(fork4_1, 0, 0x207fffff, ++nSequenceId, fork4_1.nTime + 1);
    setBlockIndexCandidates.insert(&fork4_2);
    BOOST_CHECK_EQUAL(GetPenalizedWork(fork4_1, tip->pprev->nTime).GetLow64(), 3);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), &fork4_2);

    // tip_2:  PCW = tipPCW + 2 + 8
    // A "fork" extending the tip adds a heavy late block, and another block. No penalty, receive time doesn't matter.
    // tip-2 <- tip-1 (0x207fffff)                 <- tip (0x207fffff)            <- tip_1 (0x201fffff, +1000000) <- tip_2 (0x207fffff, +1)
    //   |          \                              <- fork1_1 (0x201fffff, +0)
    //   \   <- fork2_1 (0x203fffff, tip-1 + 599)  <- fork2_2 (0x207fffff, +1)
    //   \   <- fork3_1 (0x2027ffff, tip-1 + 1800) <- fork3_2 (0x200fffff, +5400)
    //   \   <- fork4_1 (0x2027ffff, tip-1 + 600)  <- fork4_2 (0x207fffff, +1)
    CBlockIndex tip_1 = AddBlock(*tip, 0, 0x201fffff, ++nSequenceId, tip->nTime + 1000000);
    CBlockIndex tip_2 = AddBlock(tip_1, 0, 0x207fffff, ++nSequenceId, tip_1.nTime + 1);
    setBlockIndexCandidates.insert(&tip_2);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), &tip_2);

    // fork2:  PCW = tipPCW - 2 + (6 @ penalty 2^2) + (16 @ penalty 2^5) = tipPCW - 1
    // Extend fork3 so it would win, except it is very late. It is penalized to below tipPCW so cannot be chosen.
    // tip-2 <- tip-1 (0x207fffff)                 <- tip (0x207fffff)            <- tip_1 (0x201fffff, +1000000) <- tip_2 (0x207fffff, +1)
    //   |          \                              <- fork1_1 (0x201fffff, +0)
    //   \   <- fork2_1 (0x203fffff, tip-1 + 599)  <- fork2_2 (0x207fffff, +1)
    //   \   <- fork3_1 (0x2027ffff, tip-1 + 1800) <- fork3_2 (0x200fffff, +5400) <- fork3_3 (0x207fffff, +1)
    //   \   <- fork4_1 (0x2027ffff, tip-1 + 600)  <- fork4_2 (0x207fffff, +1)
    CBlockIndex fork3_3 = AddBlock(fork3_2, 0, 0x207fffff, ++nSequenceId, fork2_2.nTime + 1);
    setBlockIndexCandidates.insert(&fork3_3);
    BOOST_CHECK_EQUAL(GetPenalizedWork(fork3_2, tip->pprev->nTime).GetLow64(), 0);
    BOOST_CHECK_EQUAL(FindMostWorkChain(), &tip_2);
}
}

BOOST_AUTO_TEST_SUITE_END();


BOOST_FIXTURE_TEST_SUITE(cchain_tests, BasicTestingSetup);

BOOST_AUTO_TEST_CASE(chaintip_observer) {

    CChain chain;

    const CBlockIndex* oldTip = nullptr;
    const CBlockIndex* newTip = nullptr;
    int callbacks = 0;

    chain.AddTipObserver([&](const CBlockIndex* o, const CBlockIndex* n) {
            oldTip = o;
            newTip = n;
            ++callbacks;
        });

    CBlockIndex a, b;
    a.nHeight = 0;
    b.nHeight = 1;
    b.pprev = &a;

    chain.SetTip(&b);
    BOOST_CHECK(oldTip == nullptr);
    BOOST_CHECK(newTip == &b);
    BOOST_CHECK_EQUAL(1, callbacks);

    CBlockIndex c;
    b.nHeight = 0;
    chain.SetTip(&c);
    BOOST_CHECK(oldTip == &b);
    BOOST_CHECK(newTip == &c);
    BOOST_CHECK_EQUAL(2, callbacks);

    chain.SetTip(nullptr);
    BOOST_CHECK(oldTip == &c);
    BOOST_CHECK(newTip == nullptr);
    BOOST_CHECK_EQUAL(3, callbacks);
}

BOOST_AUTO_TEST_CASE(tip_max_blocksize) {
    CChain chain;

    // no tip set
    BOOST_CHECK_EQUAL(MAX_BLOCK_SIZE, chain.MaxBlockSizeInsecure());

    // set tips
    CBlockIndex a;
    a.nHeight = 0;
    a.nMaxBlockSize = MAX_BLOCK_SIZE * 1;

    CBlockIndex b;
    b.nHeight = 1;
    b.nMaxBlockSize = MAX_BLOCK_SIZE * 2;
    b.pprev = &a;

    CBlockIndex c;
    c.nHeight = 0;
    c.nMaxBlockSize = MAX_BLOCK_SIZE * 3;

    chain.SetTip(&b);
    BOOST_CHECK_EQUAL(2 * MAX_BLOCK_SIZE, chain.MaxBlockSizeInsecure());
    chain.SetTip(&c);
    BOOST_CHECK_EQUAL(3 * MAX_BLOCK_SIZE, chain.MaxBlockSizeInsecure());
}

BOOST_AUTO_TEST_SUITE_END();
