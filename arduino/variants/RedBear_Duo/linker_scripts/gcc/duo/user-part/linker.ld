
MEMORY
{
    APP_FLASH (rx) : ORIGIN = 0x080C0000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000300, LENGTH = 0x20000 - 0x300 - 58K
    INCLUDE backup_ram_memory.ld
}

INCLUDE module_system_part1_export.ld
INCLUDE module_system_part2_export.ld

INCLUDE ../../shared/stm32f2xx/user.ld
