#!/bin/sh

board_path=~/Library/Arduino15/packages/RedBear/hardware/STM32F2
ver=$(ls $board_path | grep 0.2.)

if [ -s $board_path/$ver ]
then

echo Will copy to $board_path/$ver

rm -r $board_path/$ver

mkdir $board_path/$ver

cp -a arduino/ $board_path/$ver/

else

echo Error: please install board package for the Duo first.

fi
