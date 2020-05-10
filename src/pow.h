// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include "consensus/params.h"
#include "arith_uint256.h"

#include <stdint.h>

class CBlockIndex;
class uint256;
class arith_uint256;

#define RTT_RETARGET 886 / 1000
static const uint32_t MAX_BLOCKSECOND = 172800;
static const arith_uint256 RTT_CONSTANT = UintToArith256(uint256S("a099408f761"));
static const uint256 MAX_HASH = uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");


unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, uint32_t blocktime, const Consensus::Params&, bool checkOverflow = true);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
arith_uint256 GetSubTarget(const arith_uint256 &bnTarget, uint32_t blocksecond, bool checkOverflow = true);
bool CheckProofOfWork(uint256 hash, unsigned int nBits, uint32_t blocktime, const Consensus::Params&, bool fLogHighHash = true);
arith_uint256 BitsToWork(unsigned int nBits);
arith_uint256 GetBlockProof(const CBlockIndex& block);
arith_uint256 GetPenalizedWork(const CBlockIndex& block, int64_t activeForkStartTime);

/** Return the time it would take to redo the work difference between from and to, assuming the current hashrate corresponds to the difficulty at tip, in seconds. */
int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params&);

#endif // BITCOIN_POW_H
