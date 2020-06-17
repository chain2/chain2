#!/bin/bash

#  Shell script that converts gitian built binaries into a simple DPKG, which can then be put into an apt repo.
#
#  Improvement ideas:
#  - Install the man page from the source repo.
#  - Wrap in a script that sends crash reports/core dumps to some issue tracker.
#  - etc ...

ver=0.11.0-O
realver=0.11O

set +e

# Make working space
workdir=chain2-$realver
[ -d $workdir ] && rm -r $workdir
mkdir $workdir
cd $workdir

# Extract the tarball to a directory called usr
tarball=chain2-$ver-linux64.tar.gz
tar xzvf ../$tarball
mv chain2-$ver usr

# copy chain2d.service file to lib/systemd/system directory
mkdir -p lib/systemd/system 
cp ../chain2d.service lib/systemd/system

# copy bitcoin.conf file to etc/chain2
mkdir -p etc/chain2
cp ../bitcoin.conf etc/chain2

# create file to force creation of data folder
mkdir -p var/lib/chain2
touch var/lib/chain2/.empty

# Rename the binaries so we don't conflict with regular Bitcoin
mv usr/bin/bitcoind usr/bin/chain2d
mv usr/bin/bitcoin-cli usr/bin/chain2-cli
mv usr/bin/bitcoin-tx usr/bin/chain2-tx
mv usr/bin/bitcoin-qt usr/bin/chain2-qt

# Remove unneeded files 
rm usr/bin/test_bitcoin
rm usr/bin/test_bitcoin-qt
rm usr/include/*
rm usr/lib/*

# Set up debian metadata. There are no dependencies beyond libc and other base DSOs as everything is statically linked.

mkdir DEBIAN
cat <<EOF >DEBIAN/control
Package: chain2
Architecture: amd64
Description: chain2 is a fully verifying Bitcoin node implementation.
Maintainer: TBD <>
Version: $realver
Depends: debconf, adduser
Recommends: ntp
EOF

cat <<EOF >DEBIAN/install
usr/bin/chain2d usr/bin
usr/bin/chain2-cli usr/bin
usr/bin/chain2-tx usr/bin
EOF

cat <<EOF >DEBIAN/conffiles
lib/systemd/system/chain2d.service
etc/chain2/bitcoin.conf
var/lib/chain2/.empty
EOF

# copy templates file to DEBIAN/templates
cp ../templates DEBIAN/templates

# copy the postinst file to DEBIAN/postinst
cp ../postinst DEBIAN/postinst
chmod 0755 DEBIAN/postinst 

# copy the prerm file to DEBIAN/prerm
cp ../prerm DEBIAN/prerm
chmod 0755 DEBIAN/prerm 

# copy the postrm file to DEBIAN/postrm
cp ../postrm DEBIAN/postrm
chmod 0755 DEBIAN/postrm

cd ..

# Build deb
dpkg-deb --build $workdir
