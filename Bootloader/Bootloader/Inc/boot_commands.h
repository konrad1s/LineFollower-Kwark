#ifndef __BOOT_COMMANDS_H__
#define __BOOT_COMMANDS_H__

#include "scp.h"

#define BOOT_CMD_NUMBERS    5U

enum Boot_ScpCommands
{
    BOOT_CMD_GET_VERSION    = 0x0001,
    BOOT_CMD_ERASE_APP      = 0x0002,
    BOOT_CMD_FLASH_DATA     = 0x0003,
    BOOT_CMD_FLASH_MAC      = 0x0004,
    BOOT_CMD_VALIDATE_APP   = 0x0005,
};

extern const SCP_Command_T bootScpCommands[BOOT_CMD_NUMBERS];

#endif /* __BOOT_COMMANDS_H__ */
