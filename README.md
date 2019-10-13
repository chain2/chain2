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
With [slight modifications to ckpool](https://bitbucket.org/dgenr8/ckpool/commits/05f073b5d8ad336b00c25fab5246c796749666ce), it can allow mining chain2 in a reasonable way with no stratum client modifications needed.  But RTT will benefit most from a bit more specialized software that doesn't want a new template each second.

Community
---------------------
https://gitter.im/chain2/community


Notably NOT Supported
---------------------
Running a wallet on a pruned node, HD wallets, Obfuscated chainstate, ZMQ
