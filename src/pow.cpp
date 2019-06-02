// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "options.h"
#include "uint256.h"
#include "util.h"

/**
 * Compute the next required proof of work using weighted-blocktime exponential moving average
 */
unsigned int GetNextWorkRequired(const CBlockIndex *pindexPrev,
                             uint32_t nextblocktime,
                             const Consensus::Params &params) {

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const uint32_t nPowLimit = bnPowLimit.GetCompact();

    if(pindexPrev == NULL || pindexPrev->pprev == NULL) {
        return nPowLimit;
    }

    if (params.fPowNoRetargeting) {
        return pindexPrev->nBits;
    }

    int64_t nBlockTime = pindexPrev->GetBlockTime() - pindexPrev->pprev->GetBlockTime();

    //Constrain individual target changes to [/1.1, *1.1]
    const int64_t upperLimit = params.nPowTargetSpacing + params.DifficultyAdjustmentSpace() / 10;
    if(nBlockTime > upperLimit) {
        nBlockTime = upperLimit;
    } else {
        const int64_t lowerLimit = params.nPowTargetSpacing - params.DifficultyAdjustmentSpace() / 11;
        if(nBlockTime < lowerLimit) {
            nBlockTime = lowerLimit;
        }
    }

    arith_uint256 bnPriorTarget;
    bnPriorTarget.SetCompact(pindexPrev->nBits);

    arith_uint256 bnNextTarget;
    bnNextTarget  = (bnPriorTarget + params.DifficultyAdjustmentSpace() - 1) /
                    params.DifficultyAdjustmentSpace();
    bnNextTarget *= params.DifficultyAdjustmentSpace() + nBlockTime - params.nPowTargetSpacing;

    if (params.fPowAllowMinDifficultyBlocks) {
        // Special difficulty rule for testnet:
        // If the new block's timestamp is more than 2* 10 minutes then allow
        // mining of a min-difficulty block.
        if (nextblocktime >
            pindexPrev->GetBlockTime() + 2 * params.nPowTargetSpacing) {
            return nPowLimit;
        }

        // Return the last non-special-min-difficulty-rules-block
        const CBlockIndex *pindex = pindexPrev;
        while (pindex->pprev &&
               pindex->nBits == nPowLimit) {
            pindex = pindex->pprev;
        }

        return pindex->nBits;
    }

    if (bnNextTarget > bnPowLimit)
        bnNextTarget = bnPowLimit;

    uint32_t nNextTarget = bnNextTarget.GetCompact();

    LogPrintf("RETARGET: Before: %08x, nBlockTime = %d, After: %08x\n", pindexPrev->nBits, nBlockTime, nNextTarget);

    return nNextTarget;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}
