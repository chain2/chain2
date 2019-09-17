// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparamsbase.h"

#include "tinyformat.h"
#include "util.h"

#include <assert.h>

const std::string CBaseChainParams::MAIN = "main";
const std::string CBaseChainParams::TESTNET = "test";
const std::string CBaseChainParams::TESTNET2 = "test2";
const std::string CBaseChainParams::REGTEST = "regtest";

void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp)
{
    strUsage += HelpMessageGroup(_("Chain selection options:"));
    strUsage += HelpMessageOpt("-testnet", _("Use the test chain"));
    if (debugHelp) {
        strUsage += HelpMessageOpt("-regtest", "Enter regression test mode, which uses a special chain in which blocks can be solved instantly. "
                                   "This is intended for regression testing tools and app development.");
    }
}

/**
 * Main network
 */
class CBaseMainParams : public CBaseChainParams
{
public:
    CBaseMainParams()
    {
        nRPCPort = 9392;
    }
};
static CBaseMainParams mainParams;

/**
 * Testnet
 */
class CBaseTestNetParams : public CBaseChainParams
{
public:
    CBaseTestNetParams()
    {
        nRPCPort = 19392;
        strDataDir = "testnet";
    }
};
static CBaseTestNetParams testNetParams;

/**
 * Testnet2
 */
class CBaseTestNet2Params : public CBaseChainParams
{
public:
    CBaseTestNet2Params()
    {
        nRPCPort = 29392;
        strDataDir = "testnet2";
    }
};
static CBaseTestNet2Params testNet2Params;

/*
 * Regression test
 */
class CBaseRegTestParams : public CBaseChainParams
{
public:
    CBaseRegTestParams()
    {
        nRPCPort = 19392;
        strDataDir = "regtest";
    }
};
static CBaseRegTestParams regTestParams;

/*
 * Unit test
 */
class CBaseUnitTestParams : public CBaseMainParams
{
public:
    CBaseUnitTestParams()
    {
        strDataDir = "unittest";
    }
};
static CBaseUnitTestParams unitTestParams;

static CBaseChainParams* pCurrentBaseParams = 0;

const CBaseChainParams& BaseParams()
{
    assert(pCurrentBaseParams);
    return *pCurrentBaseParams;
}

CBaseChainParams& BaseParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::TESTNET2)
        return testNet2Params;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectBaseParams(const std::string& chain)
{
    pCurrentBaseParams = &BaseParams(chain);
}

std::string ChainNameFromCommandLine()
{
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);
    bool fTestNet2 = GetBoolArg("-testnet2", false);

    if (int(fTestNet) + int(fRegTest) + int(fTestNet2) > 1)
        throw std::runtime_error("Only one of (-regtest, -testnet, -testnet2) may be specified.");
    if (fRegTest)
        return CBaseChainParams::REGTEST;
    if (fTestNet)
        return CBaseChainParams::TESTNET;
    if (fTestNet2)
        return CBaseChainParams::TESTNET2;
    return CBaseChainParams::MAIN;
}

bool AreBaseParamsConfigured()
{
    return pCurrentBaseParams != NULL;
}
