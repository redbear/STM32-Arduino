#!/bin/sh


mkdir 0.2.4

cp -a arduino/ 0.2.4/

tar -zcvf  duo_board_v024-beta.tar.gz 0.2.4

rm -r 0.2.4

