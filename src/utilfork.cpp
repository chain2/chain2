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

static bool IsForkActivatingBlock(int64_t mtpActivationTime,
                                  int64_t mtpCurrent, const CBlockIndex* pindexPrev)
{
    if (!mtpActivationTime)
        return false;

    if (mtpCurrent < mtpActivationTime)
        return false;

    if (pindexPrev == nullptr)
        // we activated at genesis (happens in regtest)
        return true;

    return pindexPrev->GetMedianTimePast() < mtpActivationTime;
}

// Check if this block activates UAHF. The next block is fork block and must be > 1MB.
bool IsUAHFActivatingBlock(int64_t mtpCurrent, const CBlockIndex* pindexPrev) {
    return IsForkActivatingBlock(Opt().UAHFTime(), mtpCurrent, pindexPrev);
}

bool IsThirdHFActivatingBlock(int64_t mtpCurrent, const CBlockIndex* pindexPrev) {
    return IsForkActivatingBlock(Opt().ThirdHFTime(), mtpCurrent, pindexPrev);
}

static bool IsForkActive(uint64_t mtpActivation, uint64_t mtpChainTip) {
    if (!mtpActivation)
        return false;

    return mtpChainTip >= mtpActivation;
}

bool IsUAHFActive(uint64_t mtpChainTip) {
    return IsForkActive(Opt().UAHFTime(), mtpChainTip);
}

bool IsThirdHFActive(int64_t mtpChainTip) {
    return IsForkActive(Opt().ThirdHFTime(), mtpChainTip);
}

// Check if fork with incompatible transactions is deactivated.
//
// Called when tip is being rollbacked. Rollback happens by reorg or
// invalidateblock call.
static bool NeedsClearAfterRollback(const CBlockIndex* oldTip) {
    assert(oldTip);
    if (oldTip->pprev == nullptr)
        return false;

    const int64_t mtpOld = oldTip->GetMedianTimePast();
    const int64_t mtpOldPrev = oldTip->pprev->GetMedianTimePast();

    // forks requiring mempool clearing in rollback
    const std::vector<std::function<bool(int64_t)>> forkChecks = {
        IsUAHFActive, // adds replay protection
        IsThirdHFActive}; // adds new opcodes

    for (auto isActive : forkChecks) {
        bool forkUndone = isActive(mtpOld) && !isActive(mtpOldPrev);
        if (forkUndone)
            return true;
    }

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
    int64_t mtpNew = newTip->GetMedianTimePast();

    // forks requiring mempool clearing going into fork
    const std::vector<std::function<bool(int64_t, const CBlockIndex*)>> forkChecks = {
        IsUAHFActivatingBlock // adds replay protection
    };

    for (auto& activatesFork : forkChecks) {
        if (activatesFork(mtpNew, oldTip)) {
            return true;
        }
    }

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
