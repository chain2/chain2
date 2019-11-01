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
 * Compute nBits. It will be hashed into the CURRENT block and used as the
 * basis of the search for the NEXT block.
 */
unsigned int GetNextWorkRequired(const CBlockIndex *pindexPrev, uint32_t blocktime,
                             const Consensus::Params &params, bool checkOverflow) {

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const uint32_t nPowLimit = bnPowLimit.GetCompact();

    if(pindexPrev == NULL) {
        return nPowLimit;
    }

    if (params.fPowNoRetargeting) {
        return pindexPrev->nBits;
    }

    uint32_t blocksecond = blocktime - pindexPrev->GetBlockTime();
    uint32_t t = std::min(blocksecond, MAX_BLOCKSECOND);
    uint64_t t2 = t * t;
    arith_uint256 t6 = t2;
    t6 = t6 * t6 * t6;

    arith_uint256 bnPriorTarget;
    bnPriorTarget.SetCompact(pindexPrev->nBits);

    arith_uint256 bnNextTarget = bnPriorTarget / (params.nPowTargetSpacing * RTT_RETARGET);
    arith_uint256 factor1 = bnNextTarget / RTT_CONSTANT / 6;
    bnNextTarget = factor1 * t6;

    if (checkOverflow) {
        if (bnNextTarget / t6 != factor1)
            bnNextTarget = bnPowLimit;
    }

    if (bnNextTarget > bnPowLimit)
        bnNextTarget = bnPowLimit;

    uint32_t nNextTarget = bnNextTarget.GetCompact();

    return nNextTarget;
}

arith_uint256 GetSubTarget(const arith_uint256 &bnTarget, uint32_t blocksecond, bool checkOverflow)
{
    uint32_t t = std::min(blocksecond, MAX_BLOCKSECOND);
    uint64_t t2 = t * t;
    arith_uint256 t5 = t2;
    t5 = t5 * t5 * t;

    arith_uint256 factor1 = bnTarget / RTT_CONSTANT;
    arith_uint256 bnSubTarget = factor1 * t5;

    if (checkOverflow) {
        if (bnSubTarget / t5 != factor1)
            return UintToArith256(MAX_HASH);
    }

    return bnSubTarget;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, uint32_t blocksecond,
                      const Consensus::Params& params, bool fLogHighHash)
{
    if (blocksecond == 0)
        return error("CheckProofOfWork(): RTT blocksecond = 0");

    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    arith_uint256 bnSubTarget = GetSubTarget(bnTarget, blocksecond);

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnSubTarget) {
        if (fLogHighHash) {
            LogPrint(Log::BLOCK,                     "       hash: %s\n"
                                 "                      subtarget: %s\n"
                                 "                    blocksecond: %u\n",
                    hash.GetHex(), bnSubTarget.GetHex(), blocksecond);
            return error("CheckProofOfWork(): hash doesn't match subtarget");
        } else {
            return false;
        }
    }

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
