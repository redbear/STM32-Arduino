# Board Support Package for Arduino

## Current Version

v0.2.5

* For the Duo firmware v0.2.2 (use "Burn Bootloader" from the Arduino IDE to update system firmware)
* Provides "Burn Bootloader" feature to update system firmware
* Fixes the bug in BLE_Peripheral example
* Fixes the bug in WiFiDdpNtpClient example
 
## Version History

v0.2.4

* For the Duo firmware v0.2.2, see the [Duo firmware](https://github.com/redbear/Duo/tree/master/firmware) for details.
* Fixes the Windows username bug.
* Fixes the export firmware bug, now export to .bin (not .hex). 
* Fixes the bug in grove sensor example.
* After compilation, the IDE now adds CRC32 to the compiled binary file (for DFU upload).
* Fixes the bug in webserver example (device ID).

v0.2.3

* For the Duo firmware v0.2.2
* Moves BLE (dynalib) to the Duo partition 2, this saves some compilation time and user part memory space.

v0.2.2

* For the Duo firmware v0.2.1
* Provides BLE in library.

