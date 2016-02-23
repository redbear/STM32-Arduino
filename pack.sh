#!/bin/sh

board_man_ver=0.2.4

board_pack_name=duo_board_v024

mkdir $board_man_ver

cp -a arduino/ $board_man_ver/

tar -zcvf  $board_pack_name.tar.gz $board_man_ver

rm -r $board_man_ver

