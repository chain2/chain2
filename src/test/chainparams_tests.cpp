// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "options.h"
#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(chainparams_tests, BasicTestingSetup);

BOOST_AUTO_TEST_CASE(check_network_magic) {
    auto magic = Params(CBaseChainParams::MAIN).NetworkMagic();
    BOOST_CHECK_EQUAL(0x85, magic[0]);
    BOOST_CHECK_EQUAL(0x84, magic[1]);
    BOOST_CHECK_EQUAL(0xea, magic[2]);
    BOOST_CHECK_EQUAL(0x9b, magic[3]);
}

BOOST_AUTO_TEST_SUITE_END();
