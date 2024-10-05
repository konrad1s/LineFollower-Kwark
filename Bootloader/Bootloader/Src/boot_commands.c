#include "boot_commands.h"
#include "boot.h"
#include <string.h>

static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context);
static void Boot_EraseAppCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashDataCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashMacCmd(const SCP_Packet *const packet, void *context);
static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context);


const SCP_Command_T bootScpCommands[BOOT_CMD_NUMBERS] = {
     {BOOT_CMD_GET_VERSION,     0,  Boot_GetVersionCmd},
     {BOOT_CMD_ERASE_APP,       0,  Boot_EraseAppCmd},
     {BOOT_CMD_FLASH_DATA,      0,  Boot_FlashDataCmd},
     {BOOT_CMD_FLASH_MAC,       0,  Boot_FlashMacCmd},
     {BOOT_CMD_VALIDATE_APP,    0,  Boot_ValidateAppCmd},
};

static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context)
{
    if (NULL != context && packet->header.size > 0U)
    {
        memcpy(context, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_GET_VERSION);
};

static void Boot_EraseAppCmd(const SCP_Packet *const packet, void *context)
{
    if (NULL != context && packet->header.size > 0U)
    {
        memcpy(context, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_ERASE_APP);
};

static void Boot_FlashDataCmd(const SCP_Packet *const packet, void *context)
{
    if (NULL != context && packet->header.size > 0U)
    {
        memcpy(context, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_FLASH_DATA);
};

static void Boot_FlashMacCmd(const SCP_Packet *const packet, void *context)
{
    if (NULL != context && packet->header.size > 0U)
    {
        memcpy(context, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_FLASH_MAC);
};

static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context)
{
    if (NULL != context && packet->header.size > 0U)
    {
        memcpy(context, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_VALIDATE_APP);
};