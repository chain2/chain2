// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"
#include "test/test_bitcoin.h"
#include "chainparams.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_FIXTURE_TEST_SUITE(checkpoints_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(sanity)
{
    const CCheckpointData& checkpoints = Params(CBaseChainParams::MAIN).Checkpoints();
    uint256 p2327 = uint256S("0x00000000000001a766b14f5cb079a95c8787a78c5cc5ed869cc855978f370983");
    uint256 p9151 = uint256S("0x000000000000002184c3a8eb9ec40c13e6fc47ae3693207acd163386b35e4fa2");
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 2327, p2327));
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 9151, p9151));


    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(checkpoints, 2327, p9151));
    BOOST_CHECK(!Checkpoints::CheckBlock(checkpoints, 9151, p2327));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 2327+1, p9151));
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 9151+1, p2327));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate(checkpoints) >= 9151);
}

BOOST_AUTO_TEST_SUITE_END()
