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

    uint32_t nextWorkRequired = GetNextWorkRequired(block, nextBlock.nTime, params);
    BOOST_CHECK_EQUAL(nextWorkRequired, requiredWorkExpected);
    nextBlock.pprev = block;
    nextBlock.nBits = nextWorkRequired;
    nextBlock.nHeight = (block == nullptr ? 1 : block->nHeight + 1);

    return;
}

// Build a main chain from genesis and check WTEMA target along the way
BOOST_AUTO_TEST_CASE(GetNextWorkRequired_test)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const uint32_t nPowLimit = bnPowLimit.GetCompact();

    uint32_t requiredWorkExpected;
    CBlockIndex b0, b1, b2, b3, b4;

    // Genesis block
    b0.nTime = 0;
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(nullptr, b0, requiredWorkExpected, params);

    // Bump against minimum difficulty (max target)
    b1.nTime = b0.nTime + 573;
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(&b0, b1, requiredWorkExpected, params);

    // Slight target decrease
    b2.nTime = b1.nTime + 572;
    requiredWorkExpected = 0x1d00ff00;
    TestNextWorkRequired(&b1, b2, requiredWorkExpected, params);

    // Exactly 10 minutes - target increase under RTT
    b3.nTime = b2.nTime + params.nPowTargetSpacing;
    requiredWorkExpected = nPowLimit;
    TestNextWorkRequired(&b2, b3, requiredWorkExpected, params);

    // 1 second - massive target decrease
    b4.nTime = b3.nTime + 1;
    requiredWorkExpected = 0x16020c9e;
    TestNextWorkRequired(&b3, b4, requiredWorkExpected, params);
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
        blocks[i].nChainWork = (i ? blocks[i - 1].nChainWork : arith_uint256(0))
                               + GetBlockProof(blocks[i]);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[GetRand(10000)];
        CBlockIndex *p2 = &blocks[GetRand(10000)];
        CBlockIndex *p3 = &blocks[GetRand(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, params);
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();
    const int64_t T = params.nPowTargetSpacing * RTT_RETARGET;
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const uint32_t nPowLimit = bnPowLimit.GetCompact();

    // Consensus minimum difficulty
    BOOST_CHECK_EQUAL(nPowLimit, 0x1d00ffff);
    // Real-time targeting constant
    BOOST_CHECK(RTT_CONSTANT == UintToArith256(uint256S("a099408f761")));

    // Bump against minimum difficulty
    arith_uint256 overLimit = bnPowLimit * 11 / 10;
    BOOST_CHECK(!CheckProofOfWork(uint256(), overLimit.GetCompact() , T, params));

    // Parameter checks

    // blocksecond = 0
    BOOST_CHECK(!CheckProofOfWork(uint256(), nPowLimit, 0, params));
    // Negative nBits
    BOOST_CHECK(!CheckProofOfWork(uint256(), 0x1d800000, T, params));
    // nBits overflow
    BOOST_CHECK(!CheckProofOfWork(uint256(), 0x22000001, T, params));
    // 0 target
    BOOST_CHECK(!CheckProofOfWork(uint256(), 0x1d000000, T, params));

    // Valid hash (zero)
    BOOST_CHECK(CheckProofOfWork(uint256(), nPowLimit, T, params));
    // This should not pass on mainnet (which we are on)
    BOOST_CHECK(!CheckProofOfWork(MAX_HASH, nPowLimit, T, params));

    // Subtarget checks

    auto CheckSubTargetBasic = [](arith_uint256 bnTarget, uint32_t t) {
        arith_uint256 expected, result;
        result = GetSubTarget(bnTarget, t);
        expected = bnTarget / RTT_CONSTANT * t * t * t * t * t;
        BOOST_CHECK(result == expected);
    };
    // Minimum difficulty
    CheckSubTargetBasic(bnPowLimit, T);
    // Minimum difficulty, blocksecond=1
    CheckSubTargetBasic(bnPowLimit, 1);
    // Minimum difficulty, max blocksecond
    CheckSubTargetBasic(0x04000000, MAX_BLOCKSECOND - 1);
    // Minimum difficulty, ridiculous blocksecond
    BOOST_CHECK(GetSubTarget(bnPowLimit, MAX_BLOCKSECOND * 10000) ==
                GetSubTarget(bnPowLimit, MAX_BLOCKSECOND));
    // (At regtest min difficulty, target needs to be capped at MAX_HASH)
    arith_uint256 bnRegtestLimit;
    bnRegtestLimit.SetCompact(0x207FFFFF);
    BOOST_CHECK(GetSubTarget(bnRegtestLimit, MAX_BLOCKSECOND) == UintToArith256(MAX_HASH));
    BOOST_CHECK(GetSubTarget(bnRegtestLimit, 467) == UintToArith256(MAX_HASH));
    BOOST_CHECK(GetSubTarget(bnRegtestLimit, 466) == bnRegtestLimit / RTT_CONSTANT * 466 * 466 * 466 * 466 * 466);

}

BOOST_AUTO_TEST_SUITE_END()
