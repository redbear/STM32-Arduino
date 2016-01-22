# Firmware Management


## Install DFU-UTIL

* [DFU Installation Guide](https://github.com/redbear/Duo/blob/master/docs/dfu.md).


## Updating firmware

Press and hold the SETUP button on the Duo and then press reset button, when the RGB LED shows yellow and flashing, release the SETUP button.

From the command line:

To update System-Part1

$ dfu-util -d 2b04:d058 -a 0 -s 0x08020000 -D duo-system-part1.bin

To update System-Part2

$ dfu-util -d 2b04:d058 -a 0 -s 0x08040000 -D duo-system-part2.bin

Optional:

To update User-Part (if you want to test the board)

$ dfu-util -d 2b04:d058 -a 0 -s 0x080C0000 -D duo-user-part_blink.bin

After updating the firmware, press the reset button to run the new firmware. You should see the LED (D7) is blinking if you loaded the user part binary.

