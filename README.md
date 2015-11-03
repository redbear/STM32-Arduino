
# STM32-Arduino

Allows to use the Arduino IDE to develop STM32 MCU firmware. Currently, it supports the RedBear Duo (STM32F205).

The RedBear IoT kit has two boards, the RedBear Duo and the RBLink. The RBLink is for loading firmware to the Duo and provides interface for Seeed Studio Grove System components.

For more details for the Duo and the RBLink, read this:

https://github.com/Cheong2K/WICED-SDK

# Update Firmware

1. Stack the Duo on top of the RBLink.

2. Use the "RBDuo_Unlock.bin" to unlock the board using the RBLink, drag and drop the bin to the RBLink drive after connect it to your computer via the USB port of the RBLink.

3. The green light will flash very fast.

4. Drag and drop the RBDuo_ES2-Arduino.bin to the RBLink.

5. Remove the Duo from the RBLink.
 
# Install Driver (only for Windows)

1. USB CDC

 Connect the Duo to your Windows PC using the USB port and install the driver from the "driver/windows" folder.

2. RBLink

Based on ST-Link:

 http://www.st.com/web/en/catalog/tools/PF260219

# Setup Arduino IDE

Step 1:

Download the Arduino IDE, tested with 1.6.5 (OSX and Windows only, Linux should also work).

https://www.arduino.cc/en/Main/Software

Step 2:

Start the IDE and from the menu, Preferences, add the following to "Additional Boards Manager URLs"

https://redbearlab.github.io/arduino/package_redbear_index.json

Step 3:

From the menu, Tools > Board, select "Boards Manager" and install the RedBear Duo to the IDE.

Step 4:

Connect the Duo to your computer through the USB port of the Duo.

*** Note that, it is not the RBLink's USB port if you are going to use the RBLink for using Grove System components.

Step 5:

From the menu, Tools > Board, select RedBear Duo under RedBear IoT Boards.

Step 5:

Select the Port under Tools menu.

Step 6:

From the menu, File > Examples, select the example "Duo Blink" and upload to the board.

Step 7:

The LED on the board is blinking.

# Known Issues

1. There are two examples require to associate with an AP or a router in order to function, but if the board fail to join any, it will continue to attempt and will no longer be able to upload any sketch.

Workaround: Flash the default firmware (RBDuo_ES2-Arduino.bin) using the RBLink to the board to reset it to the default state at this moment.

