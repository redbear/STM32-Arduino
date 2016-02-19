#!bash

# This tool is for creating user firmware with CRC

pwd

# Path of crc related commands.
comand_path=$2
#echo ${comand_path}

# Path of arm-none-eabi.
gcc_path=$3
echo ${gcc_path}

# Jump to the path of crc commands.
cd ${comand_path}

# Calculate CRC32.
if [ -s $1 ]; then \
./head -c $((`./stat --print %s $1` - 38)) $1 > $1.no_crc && \
./tail -c 38 $1 > $1.crc_block && \
./test "0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20280078563412" = `./xxd -p -c 500 $1.crc_block` && \
./sha256sum $1.no_crc | ./cut -c 1-65 | ./xxd -r -p | ./dd bs=1 of=$1 seek=$((`./stat --print %s $1` - 38)) conv=notrunc  && \
./head -c $((`./stat --print %s $1` - 4)) $1 > $1.no_crc && \
./crc32 $1.no_crc | ./cut -c 3-10 | ./xxd -r -p | ./dd bs=1 of=$1 seek=$((`./stat --print %s $1` - 4)) conv=notrunc ;\
fi

# Generate hex file.
${gcc_path}/arm-none-eabi-objcopy -O ihex -I binary $1 $1.hex	