@echo off   

rem System firmware update script for the RedBear Duo   

rem Update System-Part1
%1 -d 2b04:d058 -a 0 -s 0x08020000 -D %2/duo-system-part1-v0.3.1.bin  
        
rem Update System-Part2  
%1 -d 2b04:d058 -a 0 -s 0x08040000 -D %2/duo-system-part2-v0.3.1.bin  
     
rem Update User-Part   
%1 -d 2b04:d058 -a 0 -s 0x080C0000 -D %2/duo-user-part.bin    
   
rem Update Factory Reset Part and exit DFU mode
%1 -d 2b04:d058 -a 2 -s 0x140000:leave -D %2/duo-factory-reset.bin  
