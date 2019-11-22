<pre>
  Layer: Relay Policy
  Title: Superstandard Transactions
  Author: Tom Harding &lt;tomh@thinlink.com&gt;
  Created: 2019-09-17
  License: BSD-2-Clause
</pre>

Superstandard Transactions
==========================
Abstract
--------
New and changed relay and transaction selection behaviors are specified for full node implementations. Reliability of 0-confirmation payments is increased by creating network conditions where the "first seen" policy rule can work as well as possible.

Payments will become more reliable under the following assumptions:
- The payment is a [Superstandard (SS) transaction](#specification)
- Payment recipient *monitors for respend* for several seconds before accepting payment
- Payment recipient, miner, and a large part of the network run software *implementing* this proposal

Non-SS payments are unreliable until confirmed.  This is not a problem when this proposal is implemented on a new blockchain.

Definitions
-----------
#### Respend
Define a **respend** as an unconfirmed transaction that spends one or more UTXOs also spent by another unconfirmed transaction, which differs from it even when scriptSig contents are ignored.  We do not consider a malleability clone to be a respend.

#### Time
Define absolute and relative ("later" etc.) time of a block or transaction to be the *time complete data is received*.  This is an observation independent of local verification speed.

Motivation
----------
There are two main cases of 0-confirmation double-spend that can be successful *even when miners follow a simplistic first-seen policy*. These are the [fast respend](#fast-respend) and the [reverse respend](#reverse-respend).  This proposal improves on earlier techniques in handling each case.

In the cases below, further define a **legitimate transaction** to pay a recipient who is expected to provide value in exchange for payment. An **economic double spend** occurs when a payment recipient relies on an unconfirmed transaction that is ultimately invalidated by the confirmation of a respend of that transaction.

#### Fast Respend
A **fast respend** is transmitted simultaneously with the legitimate transaction.  With a simplistic first-seen policy, miner and merchant experiences of first-seen are random, leading to probabilistic success in creating an economic double-spend.

A variant is to broadcast the respend immediately after the merchant approves payment, hoping the legitimate payment will be slowed by network randomness on its way to miners.

**Solution:** The network must propagate SS transactions, and first-respends of SS transactions, immediately.  By monitoring for respends and not seeing any, recipients can quickly gain confidence that miners have seen the same transaction first.

#### Reverse Respend
A **reverse respend** is a nonstandard respend transaction placed with miners *before* the legitimate tx is broadcast.  With a simplistic first-seen policy, miner ignores the legitimate tx and confirms the non-legitimate transaction.

**Solution:** The nonstandard respend transaction must be replaced by the legitimate transaction seen later.  This proposal specfies mempool rejection of a non-SS transaction when a conflicting SS transaction is seen.

Specification
-------------
#### Version 1.0 Superstandard (SS) Transaction
- Passes `IsStandardTx()` in chain2 0.11.0L
- Spends only confirmed UTXO's
- Is no larger than 2000 bytes
- Has not expired

#### Rule 1: SS Relay
An SS transaction is relayed immediately. It is not subject to artificial privacy or batching delays.

#### Rule 2: SS Expiration
An SS transaction tx1 expires (becomes non-SS) when
 - At least 2 blocks received later than tx1 exist in the active chain, AND
 - 1 hour has passed since the second such block was received

#### Rule 3: SS Respend Relay 
The first SS transaction seen to respend an unexpired SS transaction accepted earlier is relayed immediately.

#### Rule 4: Non-SS Replacement
A non-SS transaction shall not be mined while a conflicting unexpired SS transaction is known, unless the SS transaction respends an earlier SS transaction.

#### Rule 5: First-Seen
A later-seen respend of an SS transaction shall not be mined until the first-seen SS transaction expires.

Wallet Guidelines
----------------------
Represent unconfirmed non-SS payments as unreliable (they can be replaced by SS transactions).

Represent an unconfirmed SS payment as having increasing reliability as time passes without an observed respend of the transaction or any of its unconfirmed parents.

Discussion
----------
### SPV Wallets
SPV-serving implementations need changes to prove to BIP37 SPV wallets that the inputs of a transaction are confirmed. They can provide input merkle proofs and [unconfirmed ancestors](https://github.com/bitcoinxt/bitcoinxt/pull/139).

### Anti-DoS
Relaying respends carries a DoS risk when the respend can be much larger than the legitimate tx seen earlier.  Combined with the need for immediate relay, this requires limiting protection to relatively small standard transactions.

### Fees
In bitcoin, transaction propagation is dependent on fees.  Low fees make a successful economic double-spend more likely. This proposal aims to protect even low-fee SS transactions from economic double-spend, but no rule can guarantee that any specific transaction will be mined.  It remains the responsibility of the sender to include a fee that meets receiver's expectation for relying on the transaction while unconfirmed.
