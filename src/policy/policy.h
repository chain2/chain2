// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POLICY_H
#define BITCOIN_POLICY_H

#include "consensus/consensus.h"
#include "script/interpreter.h"
#include "script/standard.h"

#include <string>

class CCoinsViewCache;

/** The maximum size for transactions we're willing to relay/mine */
static const unsigned int MAX_STANDARD_TX_SIZE = 100000;
/** The maximum size for immediate relay and respend relay */
static const unsigned int MAX_SUPERSTANDARD_TX_SIZE = 2000;
/** Maximum number of signature check operations in an IsStandard() P2SH script */
static const unsigned int MAX_P2SH_SIGOPS = 15;
/** The maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int MAX_STANDARD_TX_SIGOPS = 4000;
/**
 * Standard script verification flags that standard transactions will comply
 * with. However scripts violating these flags may still be present in valid
 * blocks and we must accept those blocks.
 */
static const unsigned int STANDARD_SCRIPT_VERIFY_FLAGS = MANDATORY_SCRIPT_VERIFY_FLAGS |
                                                         SCRIPT_VERIFY_DERSIG |
                                                         SCRIPT_VERIFY_MINIMALDATA |
                                                         SCRIPT_VERIFY_NULLDUMMY |
                                                         SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS |
                                                         SCRIPT_VERIFY_CLEANSTACK |
                                                         SCRIPT_VERIFY_NULLFAIL |
                                                         SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY |
                                                         SCRIPT_VERIFY_CHECKSEQUENCEVERIFY |
                                                         SCRIPT_VERIFY_LOW_S |
                                                         SCRIPT_VERIFY_SIGPUSHONLY |
                                                         SCRIPT_ENABLE_SIGHASH_FORKID |
                                                         SCRIPT_ENABLE_MONOLITH_OPCODES;

/** For convenience, standard but not mandatory verify flags. */
static const unsigned int STANDARD_NOT_MANDATORY_VERIFY_FLAGS = STANDARD_SCRIPT_VERIFY_FLAGS & ~MANDATORY_SCRIPT_VERIFY_FLAGS;

/**
 * Used as the flags parameters to check for sigops as if OP_CHECKDATASIG is
 * enabled. Can be removed after OP_CHECKDATASIG is activated as the flag is
 * made standard.
 */
static const uint32_t STANDARD_CHECKDATASIG_VERIFY_FLAGS =
    STANDARD_SCRIPT_VERIFY_FLAGS | SCRIPT_ENABLE_CHECKDATASIG;


bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType);
    /**
     * Check for standard transaction types
     * @return True if all outputs (scriptPubKeys) use only standard transaction forms
     */
bool IsStandardTx(const CTransaction& tx, std::string& reason);
    /**
     * Check for standard transaction types
     * @param[in] mapInputs    Map of previous transactions that have outputs we're spending
     * @return True if all inputs (scriptSigs) use only standard transaction forms
     */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);
    /**
     * Check static superstandard criteria. Live/expired status is not checked.
     * For implementation efficiency, standardness itself is not checked here.
     * @param[in] hadNoDependencies    Had no mempool dependencies when received
     * @param[in] nTxSize    Serialized size. If 0, size will be calculated
     * @return True if all static superstandard criteria are met
     */
bool IsSuperStandardTx(const CTransaction& tx, bool hadNoDependencies, size_t nTxSize = 0);

#endif // BITCOIN_POLICY_H
