#ifndef __BOOT_COMMANDS_H__
#define __BOOT_COMMANDS_H__

#include "scp.h"

#define BOOT_CMD_NUMBERS    8U

enum Boot_ScpCommands
{
    BOOT_CMD_GET_SESSION    = 0x0007,
    BOOT_CMD_GET_VERSION    = 0xF001,
    BOOT_CMD_START_DOWNLOAD = 0xF002,
    BOOT_CMD_ERASE_APP      = 0xF003,
    BOOT_CMD_FLASH_DATA     = 0xF004,
    BOOT_CMD_FLASH_CRC      = 0xF005,
    BOOT_CMD_VALIDATE_APP   = 0xF006,
    BOOT_CMD_JUMP_TO_APP    = 0xF007,
};

extern const SCP_Command_T bootScpCommands[BOOT_CMD_NUMBERS];

#endif /* __BOOT_COMMANDS_H__ */
