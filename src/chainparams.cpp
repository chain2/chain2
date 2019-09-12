// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"
#include "versionbits.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"
#include "options.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << (int)0 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "This space for rent";
    const CScript genesisOutputScript = CScript() << OP_FALSE;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

const CMessageHeader::MessageStartChars& CChainParams::NetworkMagic() const {
    return pchMessageStart;
}

const CMessageHeader::MessageStartChars& CChainParams::DBMagic() const {
    return pchMessageStart;
}

bool CChainParams::RequireStandard() const {
    // the acceptnonstdtxn flag can only be used to narrow the behavior.
    // A blockchain whose default is to allow nonstandard txns can be configured to disallow them.
    return fRequireStandard || !GetBoolArg("-acceptnonstdtxn", true);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;

        // CHECKDATASIGVERIFY
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].name = "cdsv";
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].gbt_force = true;
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].nStartTime = 1571140800LL; // 2019-10-15T12:00:00+00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].nTimeout = 1602676800LL; // 2020-10-14T12:00:00+00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].windowsize = 12960; // ~ 90 days
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].threshold = 9720; // 75% of 12960
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].minlockedblocks = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].minlockedtime = 0;

        // testing bit
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].name = "testdummy";
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].gbt_force = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601LL; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999LL; // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].windowsize = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1916; // 95% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedblocks = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedtime = 0;

        // BIP100 max block size change critical vote position
        consensus.nMaxBlockSizeChangePosition = 1512;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd9;
        pchCashMessageStart[0] = 0xe3;
        pchCashMessageStart[1] = 0xe1;
        pchCashMessageStart[2] = 0xf3;
        pchCashMessageStart[3] = 0xe8;
        nDefaultPort = 8333;
        nPruneAfterHeight = 100000;
        nMinBlockfileBlocks = 64;

        genesis = CreateGenesisBlock(1296688602, 4294967293, 0x207fffff, VERSIONBITS_TOP_BITS, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x03c4359f68957fd2f688146b6e2beda38c937fb1bb12927f4a53792186e79a05"));
        assert(genesis.hashMerkleRoot == uint256S("0x213c74a7538c34d1d482f43690f03c85cb0cb73e5dab3a64075ab07682b0ef65"));

        vSeeds.push_back(CDNSSeedData("bitcoinabc.org", "seed.bitcoinabc.org"));
        vSeeds.push_back(CDNSSeedData("criptolayer.net", "seeder.criptolayer.net")); // criptolayer.net
        vSeeds.push_back(CDNSSeedData("bitcoinforks.org", "seed-abc.bitcoinforks.org")); // bitcoinforks seeders
        vSeeds.push_back(CDNSSeedData("bitcoinunlimited.info", "btccash-seeder.bitcoinunlimited.info")); // BU backed seeder
        vSeeds.push_back(CDNSSeedData("bitprim.org", "seed.bitprim.org")); // Bitprim
        vSeeds.push_back(CDNSSeedData("deadalnix.me", "seed.deadalnix.me")); // Amaury SÉCHET

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));
        cashaddrPrefix = "ctwo";

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = CCheckpointData();
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;

        // CHECKDATASIGVERIFY
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].name = "cdsv";
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].gbt_force = true;
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].nStartTime = 1567339200LL; // 2019-09-01T12:00:00+00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].nTimeout = 1598875200LL; // 2020-08-31T12:00:00+00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].windowsize = 4320; // ~ 30 days
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].threshold = 3240; // 75% of 4320
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].minlockedblocks = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CDSV].minlockedtime = 0;

        // testing bit
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].name = "testdummy";
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].gbt_force = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601LL; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999LL; // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].windowsize = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1916; // 95% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedblocks = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedtime = 0;

        // BIP100 max block size change critical vote position
        consensus.nMaxBlockSizeChangePosition = 1512;

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        pchCashMessageStart[0] = 0xf4;
        pchCashMessageStart[1] = 0xe5;
        pchCashMessageStart[2] = 0xf3;
        pchCashMessageStart[3] = 0xf4;
        nDefaultPort = 18333;
        nPruneAfterHeight = 1000;
        nMinBlockfileBlocks = 64;

        genesis = CreateGenesisBlock(1296688602, 414098458, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943"));
        //assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("bitcoinabc.org", "testnet-seed.bitcoinabc.org")); // Bitcoin ABC seeder
        vSeeds.push_back(CDNSSeedData("criptolayer.net","testnet-seeder.criptolayer.net")); // criptolayer.net
        vSeeds.push_back(CDNSSeedData("bitcoinforks.org", "testnet-seed-abc.bitcoinforks.org")); // bitcoinforks seeders
        vSeeds.push_back(CDNSSeedData("bitcoinunlimited.info", "testnet-seed.bitcoinunlimited.info")); // BU seeder
        vSeeds.push_back(CDNSSeedData("bitprim.org", "testnet-seed.bitprim.org")); //Bitprim
        vSeeds.push_back(CDNSSeedData("deadalnix.me", "testnet-seed.deadalnix.me")); //Amaury SÉCHET

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));
        cashaddrPrefix = "ctwotest";

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = CCheckpointData();
    }
};
static CTestNetParams testNetParams;

/**
 * Testnet for BIP100
 */
class CBIP100NetParams : public CTestNetParams {
public:
    CBIP100NetParams() {
        strNetworkID = "bip100";
        nDefaultPort = 28333;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        genesis = CreateGenesisBlock(1489351422, 3, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("6818cb3e2d0bd3e8f287093bcf0276b083084756d6c6284f39ab72cf9417c8ec"));
        //assert(genesis.hashMerkleRoot == uint256S("4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear();
        vSeeds.clear();

        checkpointData = CCheckpointData();
    }
};
static CBIP100NetParams bip100NetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.fPowNoRetargeting = true;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;

        consensus.nMaxBlockSizeChangePosition = 1512;

        // BIP135 functional tests rely on deterministic block times
        int64_t MOCKTIME = 1388534400 + (201 * 10 * 60); // Jan 1, 2014

        Consensus::DeploymentPos bip135test0 = static_cast<Consensus::DeploymentPos>(0);
        consensus.vDeployments[bip135test0].name = "bip135test0";
        consensus.vDeployments[bip135test0].gbt_force = true;
        consensus.vDeployments[bip135test0].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test0].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test0].windowsize = 100;
        consensus.vDeployments[bip135test0].threshold = 75;
        consensus.vDeployments[bip135test0].minlockedblocks = 0;
        consensus.vDeployments[bip135test0].minlockedtime = 0;

        Consensus::DeploymentPos bip135test1 = static_cast<Consensus::DeploymentPos>(1);
        consensus.vDeployments[bip135test1].name = "bip135test1";
        consensus.vDeployments[bip135test1].gbt_force = true;
        consensus.vDeployments[bip135test1].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test1].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test1].windowsize = 144;
        consensus.vDeployments[bip135test1].threshold = 108;
        consensus.vDeployments[bip135test1].minlockedblocks = 0;
        consensus.vDeployments[bip135test1].minlockedtime = 0;

        Consensus::DeploymentPos bip135test2 = static_cast<Consensus::DeploymentPos>(2);
        consensus.vDeployments[bip135test2].name = "bip135test2";
        consensus.vDeployments[bip135test2].gbt_force = true;
        consensus.vDeployments[bip135test2].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test2].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test2].windowsize = 100;
        consensus.vDeployments[bip135test2].threshold = 1;
        consensus.vDeployments[bip135test2].minlockedblocks = 0;
        consensus.vDeployments[bip135test2].minlockedtime = 0;

        Consensus::DeploymentPos bip135test3 = static_cast<Consensus::DeploymentPos>(3);
        consensus.vDeployments[bip135test3].name = "bip135test3";
        consensus.vDeployments[bip135test3].gbt_force = true;
        consensus.vDeployments[bip135test3].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test3].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test3].windowsize = 100;
        consensus.vDeployments[bip135test3].threshold = 10;
        consensus.vDeployments[bip135test3].minlockedblocks = 0;
        consensus.vDeployments[bip135test3].minlockedtime = 0;

        Consensus::DeploymentPos bip135test4 = static_cast<Consensus::DeploymentPos>(4);
        consensus.vDeployments[bip135test4].name = "bip135test4";
        consensus.vDeployments[bip135test4].gbt_force = true;
        consensus.vDeployments[bip135test4].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test4].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test4].windowsize = 100;
        consensus.vDeployments[bip135test4].threshold = 0;
        consensus.vDeployments[bip135test4].minlockedblocks = 0;
        consensus.vDeployments[bip135test4].minlockedtime = 0;

        Consensus::DeploymentPos bip135test5 = static_cast<Consensus::DeploymentPos>(5);
        consensus.vDeployments[bip135test5].name = "bip135test5";
        consensus.vDeployments[bip135test5].gbt_force = true;
        consensus.vDeployments[bip135test5].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test5].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test5].windowsize = 100;
        consensus.vDeployments[bip135test5].threshold = 95;
        consensus.vDeployments[bip135test5].minlockedblocks = 0;
        consensus.vDeployments[bip135test5].minlockedtime = 0;

        Consensus::DeploymentPos bip135test6 = static_cast<Consensus::DeploymentPos>(6);
        consensus.vDeployments[bip135test6].name = "bip135test6";
        consensus.vDeployments[bip135test6].gbt_force = true;
        consensus.vDeployments[bip135test6].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test6].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test6].windowsize = 100;
        consensus.vDeployments[bip135test6].threshold = 99;
        consensus.vDeployments[bip135test6].minlockedblocks = 0;
        consensus.vDeployments[bip135test6].minlockedtime = 0;

        Consensus::DeploymentPos bip135test7 = static_cast<Consensus::DeploymentPos>(7);
        consensus.vDeployments[bip135test7].name = "bip135test7";
        consensus.vDeployments[bip135test7].gbt_force = true;
        consensus.vDeployments[bip135test7].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test7].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test7].windowsize = 100;
        consensus.vDeployments[bip135test7].threshold = 100;
        consensus.vDeployments[bip135test7].minlockedblocks = 0;
        consensus.vDeployments[bip135test7].minlockedtime = 0;

        Consensus::DeploymentPos bip135test8 = static_cast<Consensus::DeploymentPos>(8);
        consensus.vDeployments[bip135test8].name = "bip135test8";
        consensus.vDeployments[bip135test8].gbt_force = true;
        consensus.vDeployments[bip135test8].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test8].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test8].windowsize = 10;
        consensus.vDeployments[bip135test8].threshold = 9;
        consensus.vDeployments[bip135test8].minlockedblocks = 1;
        consensus.vDeployments[bip135test8].minlockedtime = 0;

        Consensus::DeploymentPos bip135test9 = static_cast<Consensus::DeploymentPos>(9);
        consensus.vDeployments[bip135test9].name = "bip135test9";
        consensus.vDeployments[bip135test9].gbt_force = true;
        consensus.vDeployments[bip135test9].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test9].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test9].windowsize = 10;
        consensus.vDeployments[bip135test9].threshold = 9;
        consensus.vDeployments[bip135test9].minlockedblocks = 5;
        consensus.vDeployments[bip135test9].minlockedtime = 0;

        Consensus::DeploymentPos bip135test10 = static_cast<Consensus::DeploymentPos>(10);
        consensus.vDeployments[bip135test10].name = "bip135test10";
        consensus.vDeployments[bip135test10].gbt_force = true;
        consensus.vDeployments[bip135test10].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test10].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test10].windowsize = 10;
        consensus.vDeployments[bip135test10].threshold = 9;
        consensus.vDeployments[bip135test10].minlockedblocks = 10;
        consensus.vDeployments[bip135test10].minlockedtime = 0;

        Consensus::DeploymentPos bip135test11 = static_cast<Consensus::DeploymentPos>(11);
        consensus.vDeployments[bip135test11].name = "bip135test11";
        consensus.vDeployments[bip135test11].gbt_force = true;
        consensus.vDeployments[bip135test11].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test11].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test11].windowsize = 10;
        consensus.vDeployments[bip135test11].threshold = 9;
        consensus.vDeployments[bip135test11].minlockedblocks = 11;
        consensus.vDeployments[bip135test11].minlockedtime = 0;

        Consensus::DeploymentPos bip135test12 = static_cast<Consensus::DeploymentPos>(12);
        consensus.vDeployments[bip135test12].name = "bip135test12";
        consensus.vDeployments[bip135test12].gbt_force = true;
        consensus.vDeployments[bip135test12].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test12].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test12].windowsize = 10;
        consensus.vDeployments[bip135test12].threshold = 9;
        consensus.vDeployments[bip135test12].minlockedblocks = 0;
        consensus.vDeployments[bip135test12].minlockedtime = 0;

        Consensus::DeploymentPos bip135test13 = static_cast<Consensus::DeploymentPos>(13);
        consensus.vDeployments[bip135test13].name = "bip135test13";
        consensus.vDeployments[bip135test13].gbt_force = true;
        consensus.vDeployments[bip135test13].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test13].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test13].windowsize = 10;
        consensus.vDeployments[bip135test13].threshold = 9;
        consensus.vDeployments[bip135test13].minlockedblocks = 0;
        consensus.vDeployments[bip135test13].minlockedtime = 5;

        Consensus::DeploymentPos bip135test14 = static_cast<Consensus::DeploymentPos>(14);
        consensus.vDeployments[bip135test14].name = "bip135test14";
        consensus.vDeployments[bip135test14].gbt_force = true;
        consensus.vDeployments[bip135test14].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test14].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test14].windowsize = 10;
        consensus.vDeployments[bip135test14].threshold = 9;
        consensus.vDeployments[bip135test14].minlockedblocks = 0;
        consensus.vDeployments[bip135test14].minlockedtime = 9;

        Consensus::DeploymentPos bip135test15 = static_cast<Consensus::DeploymentPos>(15);
        consensus.vDeployments[bip135test15].name = "bip135test15";
        consensus.vDeployments[bip135test15].gbt_force = true;
        consensus.vDeployments[bip135test15].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test15].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test15].windowsize = 10;
        consensus.vDeployments[bip135test15].threshold = 9;
        consensus.vDeployments[bip135test15].minlockedblocks = 0;
        consensus.vDeployments[bip135test15].minlockedtime = 10;

        Consensus::DeploymentPos bip135test16 = static_cast<Consensus::DeploymentPos>(16);
        consensus.vDeployments[bip135test16].name = "bip135test16";
        consensus.vDeployments[bip135test16].gbt_force = true;
        consensus.vDeployments[bip135test16].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test16].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test16].windowsize = 10;
        consensus.vDeployments[bip135test16].threshold = 9;
        consensus.vDeployments[bip135test16].minlockedblocks = 0;
        consensus.vDeployments[bip135test16].minlockedtime = 11;

        Consensus::DeploymentPos bip135test17 = static_cast<Consensus::DeploymentPos>(17);
        consensus.vDeployments[bip135test17].name = "bip135test17";
        consensus.vDeployments[bip135test17].gbt_force = true;
        consensus.vDeployments[bip135test17].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test17].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test17].windowsize = 10;
        consensus.vDeployments[bip135test17].threshold = 9;
        consensus.vDeployments[bip135test17].minlockedblocks = 0;
        consensus.vDeployments[bip135test17].minlockedtime = 15;

        Consensus::DeploymentPos bip135test18 = static_cast<Consensus::DeploymentPos>(18);
        consensus.vDeployments[bip135test18].name = "bip135test18";
        consensus.vDeployments[bip135test18].gbt_force = true;
        consensus.vDeployments[bip135test18].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test18].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test18].windowsize = 10;
        consensus.vDeployments[bip135test18].threshold = 9;
        consensus.vDeployments[bip135test18].minlockedblocks = 10;
        consensus.vDeployments[bip135test18].minlockedtime = 10;

        Consensus::DeploymentPos bip135test19 = static_cast<Consensus::DeploymentPos>(19);
        consensus.vDeployments[bip135test19].name = "bip135test19";
        consensus.vDeployments[bip135test19].gbt_force = true;
        consensus.vDeployments[bip135test19].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test19].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test19].windowsize = 10;
        consensus.vDeployments[bip135test19].threshold = 9;
        consensus.vDeployments[bip135test19].minlockedblocks = 10;
        consensus.vDeployments[bip135test19].minlockedtime = 19;

        Consensus::DeploymentPos bip135test20 = static_cast<Consensus::DeploymentPos>(20);
        consensus.vDeployments[bip135test20].name = "bip135test20";
        consensus.vDeployments[bip135test20].gbt_force = true;
        consensus.vDeployments[bip135test20].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test20].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test20].windowsize = 10;
        consensus.vDeployments[bip135test20].threshold = 9;
        consensus.vDeployments[bip135test20].minlockedblocks = 10;
        consensus.vDeployments[bip135test20].minlockedtime = 20;

        Consensus::DeploymentPos bip135test21 = static_cast<Consensus::DeploymentPos>(21);
        consensus.vDeployments[bip135test21].name = "bip135test21";
        consensus.vDeployments[bip135test21].gbt_force = true;
        consensus.vDeployments[bip135test21].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test21].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test21].windowsize = 10;
        consensus.vDeployments[bip135test21].threshold = 9;
        consensus.vDeployments[bip135test21].minlockedblocks = 20;
        consensus.vDeployments[bip135test21].minlockedtime = 21;

        Consensus::DeploymentPos bip135test22 = static_cast<Consensus::DeploymentPos>(22);
        consensus.vDeployments[bip135test22].name = "bip135test22";
        consensus.vDeployments[bip135test22].gbt_force = true;
        consensus.vDeployments[bip135test22].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test22].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test22].windowsize = 10;
        consensus.vDeployments[bip135test22].threshold = 9;
        consensus.vDeployments[bip135test22].minlockedblocks = 21;
        consensus.vDeployments[bip135test22].minlockedtime = 20;

        Consensus::DeploymentPos bip135test23 = static_cast<Consensus::DeploymentPos>(23);
        consensus.vDeployments[bip135test23].name = "bip135test23";
        consensus.vDeployments[bip135test23].gbt_force = true;
        consensus.vDeployments[bip135test23].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test23].nTimeout = MOCKTIME + 30 + 50;
        consensus.vDeployments[bip135test23].windowsize = 10;
        consensus.vDeployments[bip135test23].threshold = 9;
        consensus.vDeployments[bip135test23].minlockedblocks = 5;
        consensus.vDeployments[bip135test23].minlockedtime = 0;

        Consensus::DeploymentPos bip135test24 = static_cast<Consensus::DeploymentPos>(24);
        consensus.vDeployments[bip135test24].name = "bip135test24";
        consensus.vDeployments[bip135test24].gbt_force = true;
        consensus.vDeployments[bip135test24].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test24].nTimeout = MOCKTIME + 30 + 50;
        consensus.vDeployments[bip135test24].windowsize = 10;
        consensus.vDeployments[bip135test24].threshold = 8;
        consensus.vDeployments[bip135test24].minlockedblocks = 5;
        consensus.vDeployments[bip135test24].minlockedtime = 0;

        // testing bit
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].name = "testdummy";
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].gbt_force = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999LL;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].windowsize = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 108; // 75% of 144
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedblocks = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].minlockedtime = 0;

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        pchCashMessageStart[0] = 0xda;
        pchCashMessageStart[1] = 0xb5;
        pchCashMessageStart[2] = 0xbf;
        pchCashMessageStart[3] = 0xfa;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
        nMinBlockfileBlocks = 4;

        genesis = CreateGenesisBlock(1296688602, 4294967293, 0x207fffff, VERSIONBITS_TOP_BITS, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x03c4359f68957fd2f688146b6e2beda38c937fb1bb12927f4a53792186e79a05"));
        assert(genesis.hashMerkleRoot == uint256S("0x213c74a7538c34d1d482f43690f03c85cb0cb73e5dab3a64075ab07682b0ef65"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = CCheckpointData();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        cashaddrPrefix = "ctworeg";
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else if (chain == CBaseChainParams::BIP100NET) {
        return bip100NetParams;
    }
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
