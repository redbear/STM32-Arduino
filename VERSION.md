# Board Support Package for Arduino

## Current Version

v0.2.4 Beta

* For the Duo firmware v0.2.2, see [firmware](firmware) folder for details.
* Fixes the Windows username bug.
* Fixes the export firmware bug, now export to .bin (not .hex). 
* Fixes the bug in grove sensor example.
* After compilation, the IDE now adds CRC32 to the compiled binary file (for DFU upload).
* Fixes the bug in webserver example (device ID).
 
## Version History

v0.2.3

* For the Duo firmware v0.2.2
* Moves BLE (dynalib) to the Duo partition 2, this saves some compilation time and user part memory space.

v0.2.2

* For the Duo firmware v0.2.1
* Provides BLE in library.

