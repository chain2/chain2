// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"
#include <map>
#include <string>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_CDSV = 0, // CHECKDATASIG
    DEPLOYMENT_TESTDUMMY = 28,
    MAX_VERSION_BITS_DEPLOYMENTS = 29
};

/**
 * Struct for each individual consensus rule change using BIP135.
 */
struct ForkDeployment
{
    /** Deployment name */
    const char *name;
    /** Whether GBT clients can safely ignore this rule in simplified usage */
    bool gbt_force;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;
    /** Window size (in blocks) for generalized versionbits signal tallying */
    int windowsize;
    /** Threshold (in blocks / window) for generalized versionbits lock-in */
    int threshold;
    /** Minimum number of blocks to remain in locked-in state */
    int minlockedblocks;
    /** Minimum duration (in seconds based on MTP) to remain in locked-in state */
    int64_t minlockedtime;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    /** Defined BIP135 deployments. */
    std::map<DeploymentPos, ForkDeployment> vDeployments;

    /**
     * BIP100: One-based position from beginning (end) of the ascending sorted list of max block size
     * votes in a 2016-block interval, at which the possible new lower (higher) max block size is read.
     * 1512 = 75th percentile of 2016
     */
    uint32_t nMaxBlockSizeChangePosition;
    uint32_t nMaxBlockSizeAdjustmentInterval;

    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
