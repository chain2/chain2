// Copyright (c) 2017 - 2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "utilfork.h"
#include "chain.h"
#include "options.h"
#include "txmempool.h"
#include "util.h"
#include "chainparams.h"
#include "versionbits.h"
#include "main.h"

// Check if fork with incompatible transactions is deactivated.
//
// Called when tip is being rolled back. Rollback happens by reorg or
// invalidateblock call.
static bool NeedsClearAfterRollback(const CBlockIndex* oldTip) {
    assert(oldTip);
    if (oldTip->pprev == nullptr)
        return false;

    // we don't track whether a clear can be avoided when a versionbit fork is rolled back
    const Consensus::Params& consensusParams = Params().GetConsensus();
    for (int i = 0; i < Consensus::MAX_VERSION_BITS_DEPLOYMENTS; i++)
    {
        Consensus::DeploymentPos bit = static_cast<Consensus::DeploymentPos>(i);
        if (IsConfiguredDeployment(consensusParams, bit))
        {
            if ((VersionBitsState(oldTip, consensusParams, bit, versionbitscache) == THRESHOLD_ACTIVE) &&
                (VersionBitsState(oldTip->pprev, consensusParams, bit, versionbitscache) != THRESHOLD_ACTIVE))
                return true;
        }
    }

    return false;
}

// Check if a fork with incompatible transactions is activated.
//
// Called when a block is appended to chain.
static bool NeedsClearAfterAppend(const CBlockIndex* oldTip, const CBlockIndex* newTip) {
    assert(oldTip);
    assert(newTip);

    // did we activate a versionbits fork which needs a mempool clear?
    const Consensus::Params& consensusParams = Params().GetConsensus();
    for (int i = 0; i < Consensus::MAX_VERSION_BITS_DEPLOYMENTS; i++)
    {
        Consensus::DeploymentPos bit = static_cast<Consensus::DeploymentPos>(i);
        if (IsConfiguredDeployment(consensusParams, bit))
        {
            if ((VersionBitsState(oldTip, consensusParams, bit, versionbitscache) != THRESHOLD_ACTIVE) &&
                (VersionBitsState(newTip, consensusParams, bit, versionbitscache) == THRESHOLD_ACTIVE) &&
                !consensusParams.vDeployments.at(bit).gbt_force)
                return true;
        }
    }

    return false;
}

void ForkMempoolClearer(
        CTxMemPool& mempool, const CBlockIndex* oldTip, const CBlockIndex* newTip)
{
    if (oldTip == nullptr || newTip == nullptr || oldTip == newTip)
        return;

    const bool rollback = oldTip->nHeight > newTip->nHeight;
    bool clear = rollback
        ? NeedsClearAfterRollback(oldTip)
        : NeedsClearAfterAppend(oldTip, newTip);

    if (!clear)
        return;

    LogPrint(Log::BLOCK, "%s - clearing mempool\n", rollback
             ? "Rollback past fork" : "Fork activating block");
    mempool.clear();
}
