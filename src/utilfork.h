// Copyright (c) 2017-2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_UTILFORK_H
#define BITCOIN_UTILFORK_H

#include <cstdint>

class CBlockIndex;
class CTxMemPool;

void ForkMempoolClearer(CTxMemPool& mempool,
                        const CBlockIndex* oldTip, const CBlockIndex* newTip);

#endif
