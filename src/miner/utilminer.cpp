#include "miner/utilminer.h"

#include "consensus/consensus.h"
#include "primitives/transaction.h"
#include "protocol.h"
#include "serialize.h"

#include <cstddef>

namespace miner {

void BloatCoinbaseSize(CMutableTransaction& coinbase) {
    // No minimum transaction size, so this is a NOP
    return;
}

} // ns miner
