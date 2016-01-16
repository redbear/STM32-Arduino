#!/bin/bash

# This tool is for creating user firmware with CRC

if [ -s $1 ]; then \
head -c $((`stat -f%z $1` - 38)) $1 > $1.no_crc && \
tail -c 38 $1 > $1.crc_block && \
test "0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20280078563412" = `xxd -p -c 500 $1.crc_block` && \
shasum -a 256 $1.no_crc | cut -c 1-65 | xxd -r -p | dd bs=1 of=$1 seek=$((`stat -f%z $1` - 38)) conv=notrunc  && \
head -c $((`stat -f%z $1` - 4)) $1 > $1.no_crc && \
 crc32 $1.no_crc | cut -c 1-10 | xxd -r -p | dd bs=1 of=$1 seek=$((`stat -f%z $1` - 4)) conv=notrunc ;\
fi

