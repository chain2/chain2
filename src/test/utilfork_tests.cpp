// Copyright (c) 2017 - 2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "chain.h"
#include "chainparams.h"
#include "options.h"
#include "test/test_bitcoin.h"
#include "utilfork.h"

#include <limits.h>
#include <iostream>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(utilfork_tests, BasicTestingSetup);

class DummyMempool : public CTxMemPool {
public:
    DummyMempool() : CTxMemPool(CFeeRate(0)) { }
    void clear() override { clearCalls++; }
    int clearCalls = 0;
};

BOOST_AUTO_TEST_CASE(forkmempoolclearer_nullptr) {
    DummyMempool mempool;
    CBlockIndex tip;
    ForkMempoolClearer(mempool, nullptr, nullptr);
    ForkMempoolClearer(mempool, &tip, nullptr);
    ForkMempoolClearer(mempool, nullptr, &tip);
    BOOST_CHECK_EQUAL(0, mempool.clearCalls);
}

BOOST_AUTO_TEST_SUITE_END()
