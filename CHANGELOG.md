# Duo: Arduino Board Package Change-log
---


## Current Version

***v0.2.10***

* For the Duo firmware v0.2.4
* Adds more comments in examples
* Changes the default system mode to `SEMI_AUTOMATIC` if using Arduino IDE
* Adds examples for demonstrating the cloud functions
* Adds examples for demonstrating external SPI flash usage
* Fixes the issue that BLE central connects to peer device failed
* Fixes the tool chain issue if another 3rd party board which also depends on the arm-none-eabi-gcc is installed
* Re-sorts the examples and rename some of them to meet the name convention


## Version History

***v0.2.9***

* For the Duo firmware v0.2.4
* Add licence in examples
* Format the examples in Arduino code convention
* Replace obscure HEX code with BLE macro definitions in examples
* Fix the issue that compilation failed if there is space in referenced path

***v0.2.8***

* For the Duo firmware v0.2.4-rc2
* Add BLE\_Central\_Multi\_Peripheral example
* Add Eddystone example
* Support updating Factory Reset Firmware via Burn Bootloader (Native USB only)
* Update other examples
* Burn Bootloader via RBLink will also burn user part (a blink application)

***v0.2.7***

* For the Duo firmware v0.2.4-rc1
* Support burn bootloader and system firmwares using the RBLink USB port
* Support burn sketch using the RBLink USB port
* Bug fixes to the examples
* BLE Controller, Chat, SimpleControls now work with the RedBear BLE Controller iOS and Android Apps
* Note: if connecting to the PC USB using the RBLink USB port, use `Serial1` instead of `Serial` (for USB) for serial debugging

***v0.2.6***

* For the Duo firmware v0.2.3
* Adds more BLE Peripheral examples
* Adds more BLE Central examples
* WebServer sketch now supports BLE

***v0.2.5***

* For the Duo firmware v0.2.2
* Provides "Burn Bootloader" feature to update system firmware
* Fixes the bug in BLE_Peripheral example
* Fixes the bug in WiFiDdpNtpClient example

***v0.2.4***

* For the Duo firmware v0.2.2
* Fixes the Windows username bug
* Fixes the export firmware bug, now export to .bin (not .hex)
* Fixes the bug in grove sensor example
* After compilation, the IDE now adds CRC32 to the compiled binary file (for DFU upload)
* Fixes the bug in webserver example (device ID)

***v0.2.3***

* For the Duo firmware v0.2.2
* Moves BLE (dynalib) to the Duo partition 2, this saves some compilation time and user part memory space

***v0.2.2***

* For the Duo firmware v0.2.1
* Provides BLE in library

