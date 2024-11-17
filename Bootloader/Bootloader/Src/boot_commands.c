/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "boot_commands.h"
#include "boot.h"
#include <string.h>


/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static void Boot_GetSession(const SCP_Packet *const packet, void *context);
static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context);
static void Boot_StartDownloadCmd(const SCP_Packet *const packet, void *context);
static void Boot_EraseAppCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashDataCmd(const SCP_Packet *const packet, void *context);
static void Boot_FlashMacCmd(const SCP_Packet *const packet, void *context);
static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context);
static void Boot_JumpToAppCmd(const SCP_Packet *const packet, void *context);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/
const SCP_Command_T bootScpCommands[BOOT_CMD_NUMBERS] = {
     {BOOT_CMD_GET_SESSION,     0,  Boot_GetSession},
     {BOOT_CMD_GET_VERSION,     0,  Boot_GetVersionCmd},
     {BOOT_CMD_START_DOWNLOAD,  0,  Boot_StartDownloadCmd},
     {BOOT_CMD_ERASE_APP,       0,  Boot_EraseAppCmd},
     {BOOT_CMD_FLASH_DATA,      0,  Boot_FlashDataCmd},
     {BOOT_CMD_FLASH_CRC,       0,  Boot_FlashMacCmd},
     {BOOT_CMD_VALIDATE_APP,    0,  Boot_ValidateAppCmd},
     {BOOT_CMD_JUMP_TO_APP,     0,  Boot_JumpToAppCmd},
};

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
static void Boot_GetSession(const SCP_Packet *const packet, void *context)
{
    Bootloader_T *const me = (Bootloader_T *const )context;
    const uint8_t responseData[] = "Bootloader ";

    SCP_Transmit(&me->scpInstance, BOOT_CMD_GET_SESSION, responseData, sizeof(responseData) - 1);
};

static void Boot_GetVersionCmd(const SCP_Packet *const packet, void *context)
{
    Bootloader_T *const me = (Bootloader_T *const )context;
    const uint8_t responseData[] = {'1', '.', '0', '0'};

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

    Boot_AddEvent(BOOT_EVENT_FLASH_CRC);
};

static void Boot_ValidateAppCmd(const SCP_Packet *const packet, void *context)
{
    Boot_AddEvent(BOOT_EVENT_VALIDATE_APP);
};

static void Boot_JumpToAppCmd(const SCP_Packet *const packet, void *context)
{
    Boot_AddEvent(BOOT_EVENT_JUMP_TO_APP);
};