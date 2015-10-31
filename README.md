
# STM32-Arduino

Allows to use the Arduino IDE to develop STM32 MCU firmware. Currently, it supports the RedBear Duo (STM32F205).

The RedBear IoT kit has two boards, the RedBear Duo and the RBLink. The RBLink is for loading firmware to the Duo and provides interface for Seeed Studio Grove System components.

# Update Firmware

1. Stack the Duo on top of the RBLink.

2. Use the "RBDuo_Unlock.bin" to unlock the board using the RBLink, drag and drop the bin to the RBLink drive after connect it to your computer via the USB port of the RBLink.

3. The green light will flash very fast.

4. Drag and drop the RBDuo_ES2-Arduino.bin to the RBLink.

5. Remove the Duo from the RBLink.
 
# Install Driver (only for Windows)

Connect the Duo to your Windows PC and install the driver from the "driver/windows" folder.

# Setup Arduino IDE

Step 1:

Download the Arduino IDE, tested with 1.6.5 (OSX and Windows only, Linux should also work).

https://www.arduino.cc/en/Main/Software

Step 2:

Start the IDE and from the menu, Preferences, add the following to "Additional Boards Manager URLs"

https://redbearlab.github.io/arduino/package_redbear_index.json

Step 3:

From the menu, Tools > Board, select RedBear Duo under RedBear IoT Boards.

Step 4:

Select the Port under Tools menu.

Step 5:

From the menu, File > Examples, select the example "Duo Blink" and upload to the board.

Step 6:

The LED on the board is blinking.
