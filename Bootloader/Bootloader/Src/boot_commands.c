#include "boot_commands.h"
#include "boot.h"
#include <string.h>

static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context);
static void Boot_StartDownloadCmd(const SCP_Packet *const packet, void *context);
static void Boot_EraseAppCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashDataCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashMacCmd(const SCP_Packet *const packet, void *context);
static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context);
static void Boot_JumpToAppCmd(const SCP_Packet *const packet, void *context);


const SCP_Command_T bootScpCommands[BOOT_CMD_NUMBERS] = {
     {BOOT_CMD_GET_VERSION,     0,  Boot_GetVersionCmd},
     {BOOT_CMD_START_DOWNLOAD,  0,  Boot_StartDownloadCmd},
     {BOOT_CMD_ERASE_APP,       0,  Boot_EraseAppCmd},
     {BOOT_CMD_FLASH_DATA,      0,  Boot_FlashDataCmd},
     {BOOT_CMD_FLASH_MAC,       0,  Boot_FlashMacCmd},
     {BOOT_CMD_VALIDATE_APP,    0,  Boot_ValidateAppCmd},
     {BOOT_CMD_JUMP_TO_APP,     0,  Boot_JumpToAppCmd},
};

static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context)
{
    const uint8_t responseData[] = {0x01, 0x00, 0x00, 0x00};
    Bootloader_T *const me = (Bootloader_T *const )context;

    SCP_Transmit(&me->scpInstance, BOOT_CMD_GET_VERSION, responseData, sizeof(responseData));
};

static void Boot_StartDownloadCmd(const SCP_Packet *const packet, void *context)
{
    Bootloader_T *const me = (Bootloader_T *const )context;

    Boot_AddEvent(BOOT_EVENT_START_DOWNLOAD);
    SCP_Transmit(&me->scpInstance, BOOT_CMD_START_DOWNLOAD, NULL, 0);
};

static void Boot_EraseAppCmd(const SCP_Packet *const packet, void *context)
{
    Boot_AddEvent(BOOT_EVENT_ERASE_APP);
};

static void Boot_FlashDataCmd(const SCP_Packet *const packet, void *context)
{
    Bootloader_T *const me = (Bootloader_T *const )context;

    if ((NULL != me) && (packet->header.size > 0U) && (packet->header.size <= BOOT_FLASH_BUFFER_SIZE))
    {
        memcpy(me->flashBuffer, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_FLASH_DATA);
};

static void Boot_FlashMacCmd(const SCP_Packet *const packet, void *context)
{
    Bootloader_T *const me = (Bootloader_T *const )context;

    if ((NULL != me) && (packet->header.size > 0U) && (packet->header.size <= BOOT_FLASH_BUFFER_SIZE))
    {
        memcpy(me->flashBuffer, packet->data, packet->header.size);
    }

    Boot_AddEvent(BOOT_EVENT_FLASH_MAC);
};

static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context)
{
    Boot_AddEvent(BOOT_EVENT_VALIDATE_APP);
};

static void Boot_JumpToAppCmd(const SCP_Packet *const packet, void *context)
{
    Boot_AddEvent(BOOT_EVENT_JUMP_TO_APP);
};