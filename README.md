chain2
======

chain2 is an experimental SHA256 altcoin designed to explore the performance of the Real-Time Targeting (RTT) difficulty adjustment algorithm ([research](/specifications/rtt.pdf), [implementation](https://github.com/chain2/chain2/pull/6)).

[Download](https://github.com/chain2/chain2/releases)
---------------------

Mining
---------------------
With [slight modifications to ckpool](https://bitbucket.org/dgenr8/ckpool/commits/05f073b5d8ad336b00c25fab5246c796749666ce), it can allow mining chain2 in a reasonable way with no stratum client modifications needed.  But RTT will benefit most from a bit more specialized software that doesn't want a new template each second.

- Pool https://gnark-mining.com<br>
-a sha256 -o stratum+tcp://gnark-mining.com:3338 -u -p c=CTWO

Resources
---------------------
- Explorer http://45.63.9.40/
- Chat https://discord.gg/pydVnNh

Consensus Rules
---------------------
- Bitcoin Cash rules as of 14 November, 2018 (PRE-SPLIT)
   - Has BIP143, 32MB blocks, etc.
- Real-Time Targeting
- CHECKDATASIG activation at 75% support over 90 days (BIP135)
- Gradual max block size adjustment by miners (BIP100)

Other Notable Features
---------------------
- XThin and compact blocks
- "ctwo:" addresses

Notably NOT Supported
---------------------
Running a wallet on a pruned node, HD wallets, obfuscated chainstate, ZMQ, Schnorr

Building
---------------------
- [Unix Build Notes](/doc/build-unix.md)
- [OSX Build Notes](/doc/build-osx.md)
