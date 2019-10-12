chain2
======

chain2 is an experimental SHA256 altcoin designed to explore the performance of the Real-Time Targeting (RTT) difficulty adjustment algorithm.

Consensus Rules
---------------------
- Bitcoin Cash rules as of 14 November, 2018 (PRE-SPLIT)
   - Has BIP143, 32MB blocks, etc.
- Real-Time Targeting (see PR#6)
- CHECKDATASIG activation at 75% support over 90 days (BIP135)
- Gradual max block size adjustment by miners (BIP100)

Other Notable Features
---------------------
- XThin and compact blocks
- "ctwo:" addresses

Building
---------------------
- [Unix Build Notes](/doc/build-unix.md)
- [OSX Build Notes](/doc/build-osx.md)

Mining
---------------------
With slight modifications to ckpool, it can allow mining chain2 in a reasonable way with no stratum client modifications needed.

Community
---------------------
https://gitter.im/chain2/community


Notably NOT Supported
---------------------
Running a wallet on a pruned node, HD wallets, Obfuscated chainstate, ZMQ
