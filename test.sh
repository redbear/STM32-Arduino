#!/bin/bash

#
#	Script for copy to arduno15 folder
#
#	For Linux root: $ sudo test.sh root 
#

os=`uname`

if [ $os == 'Darwin' ]; then
  board_path=~/Library/Arduino15/packages/RedBear/hardware/STM32F2
elif [ $os == 'Windows' ]; then
  board_path=~/Library/Arduino15/packages/RedBear/hardware/STM32F2
elif [ $os=='Linux' ]; then
#if [ $# -eq 1 ]; then
    board_path=/root/.arduino15/packages/RedBear/hardware/STM32F2
#else
#board_path=~/.arduino15/packages/RedBear/hardware/STM32F2
# fi
else
  echo Unknown OS
  exit
fi

ver=$(ls $board_path | grep 0.2.)

echo OS is $os

if [ -s $board_path/$ver ]; then
  echo Will copy to $board_path/$ver
  rm -r $board_path/$ver
  mkdir $board_path/$ver
  cp -a arduino/* $board_path/$ver/
else
  echo Error: please install board package for the Duo first.
fi
