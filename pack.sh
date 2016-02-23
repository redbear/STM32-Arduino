#!/bin/sh

sudo chmod +x arduino/tools/crc32/linux/crc32.sh
sudo chmod +x arduino/tools/crc32/linux/sh
sudo chmod +x arduino/tools/crc32/macosx/crc32.sh
sudo chmod +x arduino/tools/crc32/macosx/sh  

board_pack_name=duo_board_v$1

mkdir $1

cp -a arduino/ $1/

tar -zcvf  $board_pack_name.tar.gz $1

rm -r $1

