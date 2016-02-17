
# CRC32 Tool

For Windows, you need cygwin or msys.

In this document, we will use the file 'Duo_Blink-NO_CRC.ino.bin' as an eample to add crc32 checksum to the Arduino compiled firmware.

To check the file has CRC-32 or not:

	$ hexdump Duo_Blink-NO_CRC.ino.bin 

You will see there is an invalid CRC-32 code at the end (0x12345678):

	0000ba0 0007 0000 0ac1 080c 0000 0000 ffff ffff	0000bb0 0000 0201 0403 0605 0807 0a09 0c0b 0e0d	0000bc0 100f 1211 1413 1615 1817 1a19 1c1b 1e1d	0000bd0 201f 0028 5678 1234	0000bd8
To apply CRC-32 code, use the 'crc.sh' sell script:

	$ ./crc.sh Duo_Blink-NO_CRC.ino.bin

After that, the binary will have the crc-32 code at the end:

	0000ba0 0007 0000 0ac1 080c 0000 0000 ffff ffff	0000bb0 0000 a2e5 6962 970f 59a9 b2e5 7633 100b	0000bc0 b5df 7040 544d 1793 b707 7ba8 735c 8a98	0000bd0 48f9 0028 a6ad e5d4	0000bd8