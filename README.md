
# STM32-Arduino

Allows to use the Arduino IDE to develop STM32 MCU firmware. Currently, it supports the RedBear Duo (STM32F205).

The RedBear IoT kit has two boards, the RedBear Duo and the RBLink. The RBLink is for update firmware of the Duo.

# Update Firmware



# Installation

Step 1:

Connect 
Step 1:

Install driver if you are using Windows.



Step 2:

Download the Arduino IDE, tested with 1.6.5 (OSX and Windows only, Linux should also work).

https://www.arduino.cc/en/Main/Software

Step 3:

Start the IDE and from the menu, Preferences, add the following to "Additional Boards Manager URLs"

https://redbearlab.github.io/arduino/package_redbear_index.json

Step 4:

From the menu, Tools > Board, select RedBear Duo under RedBear IoT Boards.

Step 5:

Select the Port under Tools menu.

Step 6:

From the menu, File > Examples, select the example "Duo Blink" and upload to the board.
