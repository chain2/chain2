// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "pow.h"
#include "util.h"
#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>
#include <cmath>

using namespace std;

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/*
 * Test that GetNextWorkRequired() is correct for block, and start populating next block
 */
void TestNextWorkRequired(CBlockIndex* const block, CBlockIndex& nextBlock, uint32_t requiredWorkExpected,
        const Consensus::Params& params) {

    uint32_t nextWorkRequired = GetNextWorkRequired(block, 0, params);
    BOOST_CHECK_EQUAL(nextWorkRequired, requiredWorkExpected);
int breakpointfodder = nextWorkRequired == requiredWorkExpected;
nextBlock.nFile = breakpointfodder;
    nextBlock.pprev = block;
    if (block) {
        nextBlock.nHeight = block->nHeight + 1;
        nextBlock.nBits = nextWorkRequired;
    }

    return;
}

// Build a main chain from genesis and check WTEMA target along the way
BOOST_AUTO_TEST_CASE(get_next_work)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    const int64_t adjustmentSpace = params.DifficultyAdjustmentSpace();

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const uint32_t nPowLimit = bnPowLimit.GetCompact();

    uint32_t requiredWorkExpected;
    CBlockIndex b0, b1, b2, b3, b4, b5, b6, b7;

    // Test call with nullptr and set up genesis block
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(nullptr, b0, requiredWorkExpected, params);

    // Test genesis block
    b0.nTime = 0;
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(&b0, b1, requiredWorkExpected, params);

    // Bump against minimum difficulty
    b1.nTime = b0.nTime + params.nPowTargetSpacing + 1;
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(&b1, b2, requiredWorkExpected, params);

    // Just under 10 minutes
    b2.nTime = b1.nTime + params.nPowTargetSpacing - 1;
    arith_uint256 temp1;
    temp1.SetCompact(b2.nBits);
    temp1 -= temp1 / adjustmentSpace;
    requiredWorkExpected = temp1.GetCompact();
    TestNextWorkRequired(&b2, b3, requiredWorkExpected, params);

    // Exactly 10 minutes
    b3.nTime = b2.nTime + params.nPowTargetSpacing;
    TestNextWorkRequired(&b3, b4, requiredWorkExpected, params);

    // Move clock forward to give some room for the upcoming negative blocktime
    // test. This is fine because WTEMA never looks at timestamps further back
    // than tip-1
    b3.nTime += 6000;

    // 60 seconds
    b4.nTime = b3.nTime + 60;
    temp1.SetCompact(b4.nBits);
    temp1 -= (params.nPowTargetSpacing - 60) * (temp1 / adjustmentSpace);
    requiredWorkExpected = temp1.GetCompact();
    TestNextWorkRequired(&b4, b5, requiredWorkExpected, params);

    // Blocktime sufficiently negative to trigger lower target adjustment limit
    b5.nTime = (int64_t)b4.nTime + params.nPowTargetSpacing - adjustmentSpace / 11 - 1;
    temp1.SetCompact(b5.nBits);
    temp1 = (temp1 + adjustmentSpace - 1) / adjustmentSpace;
    temp1 *= adjustmentSpace - adjustmentSpace / 11;
    requiredWorkExpected = temp1.GetCompact();
    TestNextWorkRequired(&b5, b6, requiredWorkExpected, params);

    // A block that takes a week to mine
    b6.nTime = b5.nTime + 7 * 24 * 60 * 60;
    temp1.SetCompact(b6.nBits);
    temp1 = (temp1 + adjustmentSpace - 1) / adjustmentSpace;
    temp1 *= adjustmentSpace + adjustmentSpace / 10;
    requiredWorkExpected = temp1.GetCompact();
    TestNextWorkRequired(&b6, b7, requiredWorkExpected, params);

    // Hitting upper limit immediately after lower limit should have
    // near-zero net effect at compact precision
    int64_t drift = std::abs((int64_t)b7.nBits - (int64_t)b5.nBits);
    BOOST_CHECK_LE(drift, 1);
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : NULL;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1269211443 + i * params.nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork =
            i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i])
              : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[GetRand(10000)];
        CBlockIndex *p2 = &blocks[GetRand(10000)];
        CBlockIndex *p3 = &blocks[GetRand(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, params);
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

BOOST_AUTO_TEST_SUITE_END()
