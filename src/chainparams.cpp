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
 * Build the genesis block. Per tradition, the output of its generation
 * transaction cannot be spent.
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "0000000000000000000f978e55f9716d0be8eca66c63a9515b01bf265b624817";
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
        consensus.powLimit = uint256S("00000000ffff0000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetSpacing = 10 * 60;
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
        consensus.nMaxBlockSizeAdjustmentInterval = 2016;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x85;
        pchMessageStart[1] = 0x84;
        pchMessageStart[2] = 0xea;
        pchMessageStart[3] = 0x9b;
        nDefaultPort = 9393;
        nPruneAfterHeight = 100000;
        nMinBlockfileBlocks = 64;

        genesis = CreateGenesisBlock(1570992534, 4232613203, 0x1d00ffff, VERSIONBITS_TOP_BITS, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("000000002b9cd0a34fc068eeba6fbce5ce3313b2f691756cff431f103067ce21"));
        assert(genesis.hashMerkleRoot == uint256S("6c0441e368d12f1fcadb6fbc0de69221c06b9d00038dc533eaed107779da51fd"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vSeeds.push_back(CDNSSeedData("chain2.org", "seeds.chain2.org"));

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));
        cashaddrPrefix = "ctwo";

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (2327, uint256S("0x00000000000001a766b14f5cb079a95c8787a78c5cc5ed869cc855978f370983"))
            (9151, uint256S("0x000000000000002184c3a8eb9ec40c13e6fc47ae3693207acd163386b35e4fa2")),
            1576601976, // * UNIX timestamp of last checkpoint block
            11984,      // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            500.0       // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.powLimit = uint256S("00000000ffff0000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetSpacing = 10 * 60;
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
        consensus.nMaxBlockSizeAdjustmentInterval = 2016;

        pchMessageStart[0] = 0xad;
        pchMessageStart[1] = 0xac;
        pchMessageStart[2] = 0xef;
        pchMessageStart[3] = 0xc1;
        nDefaultPort = 19393;
        nPruneAfterHeight = 1000;
        nMinBlockfileBlocks = 64;

        // Commit to the hash of block #595306 on the most-work bitcoin chain, 2019-09-17
        const char* pszTimestamp = "0000000000000000000914a6728b2ff963775b8358b7cf87a46911b9f6c80b98";
        const CScript genesisOutputScript = CScript() << OP_FALSE;
        genesis = CreateGenesisBlock(pszTimestamp, genesisOutputScript, 1568735995, 3853159094, 0x1d00ffff, VERSIONBITS_TOP_BITS, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("00000000a268e384a952a6eb4b472b37906ee58b602bdbdc0497d8a4c17c0d4f"));
        assert(genesis.hashMerkleRoot == uint256S("7d35a43cb4e5e62ccc2d32990185194c866a8f6599a5066aec017a63b9ca8857"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("chain2.org", "seeds-test.chain2.org"));

        vFixedSeeds.clear();
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
 * Testnet2
 */
class CTestNet2Params : public CTestNetParams {
public:
    CTestNet2Params() {
        strNetworkID = "test2";
        nDefaultPort = 29393;
        consensus.powLimit = uint256S("7fffff0000000000000000000000000000000000000000000000000000000000");

        // Commit to the hash of block #595306 on the most-work bitcoin chain, 2019-09-17
        const char* pszTimestamp = "0000000000000000000914a6728b2ff963775b8358b7cf87a46911b9f6c80b98";
        const CScript genesisOutputScript = CScript() << OP_FALSE;
        genesis = CreateGenesisBlock(pszTimestamp, genesisOutputScript, 1568746371, 4294967294, 0x207fffff, VERSIONBITS_TOP_BITS, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("55b0df65cb3c7cb0c81ad8800980b0901d9bf8c3aa906326e37c4b65e3cd424e"));
        assert(genesis.hashMerkleRoot == uint256S("7d35a43cb4e5e62ccc2d32990185194c866a8f6599a5066aec017a63b9ca8857"));

        vFixedSeeds.clear();
        vSeeds.clear();

        checkpointData = CCheckpointData();
    }
};
static CTestNet2Params testNet2Params;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.powLimit = uint256S("7fffff0000000000000000000000000000000000000000000000000000000000");
        consensus.fPowNoRetargeting = true;
        consensus.nPowTargetSpacing = 10 * 60;

        consensus.nMaxBlockSizeChangePosition = 1512;
        consensus.nMaxBlockSizeAdjustmentInterval = 2016;

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
        consensus.vDeployments[bip135test13].minlockedtime = 5 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test14 = static_cast<Consensus::DeploymentPos>(14);
        consensus.vDeployments[bip135test14].name = "bip135test14";
        consensus.vDeployments[bip135test14].gbt_force = true;
        consensus.vDeployments[bip135test14].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test14].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test14].windowsize = 10;
        consensus.vDeployments[bip135test14].threshold = 9;
        consensus.vDeployments[bip135test14].minlockedblocks = 0;
        consensus.vDeployments[bip135test14].minlockedtime = 9 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test15 = static_cast<Consensus::DeploymentPos>(15);
        consensus.vDeployments[bip135test15].name = "bip135test15";
        consensus.vDeployments[bip135test15].gbt_force = true;
        consensus.vDeployments[bip135test15].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test15].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test15].windowsize = 10;
        consensus.vDeployments[bip135test15].threshold = 9;
        consensus.vDeployments[bip135test15].minlockedblocks = 0;
        consensus.vDeployments[bip135test15].minlockedtime = 10 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test16 = static_cast<Consensus::DeploymentPos>(16);
        consensus.vDeployments[bip135test16].name = "bip135test16";
        consensus.vDeployments[bip135test16].gbt_force = true;
        consensus.vDeployments[bip135test16].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test16].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test16].windowsize = 10;
        consensus.vDeployments[bip135test16].threshold = 9;
        consensus.vDeployments[bip135test16].minlockedblocks = 0;
        consensus.vDeployments[bip135test16].minlockedtime = 11 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test17 = static_cast<Consensus::DeploymentPos>(17);
        consensus.vDeployments[bip135test17].name = "bip135test17";
        consensus.vDeployments[bip135test17].gbt_force = true;
        consensus.vDeployments[bip135test17].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test17].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test17].windowsize = 10;
        consensus.vDeployments[bip135test17].threshold = 9;
        consensus.vDeployments[bip135test17].minlockedblocks = 0;
        consensus.vDeployments[bip135test17].minlockedtime = 15 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test18 = static_cast<Consensus::DeploymentPos>(18);
        consensus.vDeployments[bip135test18].name = "bip135test18";
        consensus.vDeployments[bip135test18].gbt_force = true;
        consensus.vDeployments[bip135test18].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test18].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test18].windowsize = 10;
        consensus.vDeployments[bip135test18].threshold = 9;
        consensus.vDeployments[bip135test18].minlockedblocks = 10;
        consensus.vDeployments[bip135test18].minlockedtime = 10 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test19 = static_cast<Consensus::DeploymentPos>(19);
        consensus.vDeployments[bip135test19].name = "bip135test19";
        consensus.vDeployments[bip135test19].gbt_force = true;
        consensus.vDeployments[bip135test19].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test19].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test19].windowsize = 10;
        consensus.vDeployments[bip135test19].threshold = 9;
        consensus.vDeployments[bip135test19].minlockedblocks = 10;
        consensus.vDeployments[bip135test19].minlockedtime = 19 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test20 = static_cast<Consensus::DeploymentPos>(20);
        consensus.vDeployments[bip135test20].name = "bip135test20";
        consensus.vDeployments[bip135test20].gbt_force = true;
        consensus.vDeployments[bip135test20].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test20].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test20].windowsize = 10;
        consensus.vDeployments[bip135test20].threshold = 9;
        consensus.vDeployments[bip135test20].minlockedblocks = 10;
        consensus.vDeployments[bip135test20].minlockedtime = 20 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test21 = static_cast<Consensus::DeploymentPos>(21);
        consensus.vDeployments[bip135test21].name = "bip135test21";
        consensus.vDeployments[bip135test21].gbt_force = true;
        consensus.vDeployments[bip135test21].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test21].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test21].windowsize = 10;
        consensus.vDeployments[bip135test21].threshold = 9;
        consensus.vDeployments[bip135test21].minlockedblocks = 20;
        consensus.vDeployments[bip135test21].minlockedtime = 21 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test22 = static_cast<Consensus::DeploymentPos>(22);
        consensus.vDeployments[bip135test22].name = "bip135test22";
        consensus.vDeployments[bip135test22].gbt_force = true;
        consensus.vDeployments[bip135test22].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test22].nTimeout = 999999999999LL;
        consensus.vDeployments[bip135test22].windowsize = 10;
        consensus.vDeployments[bip135test22].threshold = 9;
        consensus.vDeployments[bip135test22].minlockedblocks = 21;
        consensus.vDeployments[bip135test22].minlockedtime = 20 * consensus.nPowTargetSpacing;

        Consensus::DeploymentPos bip135test23 = static_cast<Consensus::DeploymentPos>(23);
        consensus.vDeployments[bip135test23].name = "bip135test23";
        consensus.vDeployments[bip135test23].gbt_force = true;
        consensus.vDeployments[bip135test23].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test23].nTimeout = MOCKTIME + 30 + 50 * consensus.nPowTargetSpacing;
        consensus.vDeployments[bip135test23].windowsize = 10;
        consensus.vDeployments[bip135test23].threshold = 9;
        consensus.vDeployments[bip135test23].minlockedblocks = 5;
        consensus.vDeployments[bip135test23].minlockedtime = 0;

        Consensus::DeploymentPos bip135test24 = static_cast<Consensus::DeploymentPos>(24);
        consensus.vDeployments[bip135test24].name = "bip135test24";
        consensus.vDeployments[bip135test24].gbt_force = true;
        consensus.vDeployments[bip135test24].nStartTime = MOCKTIME + 30;
        consensus.vDeployments[bip135test24].nTimeout = MOCKTIME + 30 + 50 * consensus.nPowTargetSpacing;
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

        pchMessageStart[0] = 0x8c;
        pchMessageStart[1] = 0xee;
        pchMessageStart[2] = 0xc1;
        pchMessageStart[3] = 0xe3;
        nDefaultPort = 19494;
        nPruneAfterHeight = 1000;
        nMinBlockfileBlocks = 4;

        const char* pszTimestamp = "This space for rent";
        const CScript genesisOutputScript = CScript() << OP_FALSE;
        genesis = CreateGenesisBlock(pszTimestamp, genesisOutputScript, 1296688602, 4294967293, 0x207fffff, VERSIONBITS_TOP_BITS, 50 * COIN);
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
    else if (chain == CBaseChainParams::TESTNET2) {
        return testNet2Params;
    }
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
