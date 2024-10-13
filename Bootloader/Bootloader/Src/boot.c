#include "boot.h"
#include "scp.h"
#include "boot_commands.h"
#include "crc.h"

#define BOOT_BACKDOOR_TIMEOUT 50U
#define BOOT_EXTENDED_TIMEOUT 30000U
#define BOOT_APP_RESET_FLAG 0xDEADBEEF
#define BOOT_SCP_BUFFER_SIZE 4096U

extern void Boot_JumpToApp(uint32_t address);
static void Boot_CheckAndClearBootShared(uint32_t *bootFlags);

__attribute__((section(".noinit_shared"))) static uint32_t bootFlags;
static uint8_t scpBuffer[BOOT_SCP_BUFFER_SIZE];

static Bootloader_T bootloader = {
    .state = BOOT_STATE_IDLE,
    .scpInstance = {
        .buffer = scpBuffer,
        .size = sizeof(scpBuffer),
        .huart = &huart4,
        .commands = bootScpCommands,
        .numCommands = sizeof(bootScpCommands) / sizeof(bootScpCommands[0]),
        .errorHandler = NULL},
    .flashManager = {
        .appSectorRange = {
            .startSector = FLASH_SECTOR_3,
            .sectorCount = 4U},
        .appStartAddress = 0x0800C000U,
        .appEndAddress = 0x0805FFFFU
        },
};

static bool Boot_ValidateApp(void)
{
    uint32_t appStartAddress = bootloader.flashManager.appStartAddress;
    uint32_t crcStartAddress = appStartAddress;
    uint32_t crcEndAddress = bootloader.flashManager.appEndAddress - sizeof(uint32_t);
    uint32_t crcSize = crcEndAddress - crcStartAddress + 1U;

    uint32_t crcCalculated = HAL_CRC_Calculate(&hcrc, (uint32_t *)crcStartAddress, crcSize);

    /* Don't care about endianness, the data is always in little endian */
    uint32_t storedCRC = *(uint32_t *)(bootloader.flashManager.appEndAddress - 3U);

    return (crcCalculated == storedCRC);
}


static void Boot_HandleStateIdle(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            if (bootloader.backdoorTimer < bootloader.backdoorTimerTimeout)
            {
                bootloader.backdoorTimer++;
            }
            else
            {
                /* Boot backdoor timeout, try to boot the application */
                bootloader.state = BOOT_STATE_BOOTING;
            }
            break;
        case BOOT_EVENT_START_DOWNLOAD:
            bootloader.backdoorTimer = 0U;
            bootloader.backdoorTimerTimeout = BOOT_EXTENDED_TIMEOUT;
            bootloader.state = BOOT_STATE_ERASING;
            break;
        case BOOT_EVENT_JUMP_TO_APP:
            bootloader.backdoorTimer = 0U;
            bootloader.state = BOOT_STATE_BOOTING;
            break;
        default:
            break;
    }
}

static void Boot_HandleStateBooting(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            if (bootloader.isAppValid)
            {
                Boot_JumpToApp(bootloader.flashManager.appStartAddress);
            }
            else if (true == Boot_ValidateApp())
            {
                bootloader.isAppValid = true;
                Boot_JumpToApp(bootloader.flashManager.appStartAddress);
            }
            else
            {
                /* Stay in bootloader */
                bootloader.state = BOOT_STATE_ERASING;
                bootloader.isAppValid = false;
            }
            break;
        default:
            break;
    }
}

static void Boot_HandleStateErasing(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_JUMP_TO_APP:
            /* Still jump to app is possible before erase event */
            bootloader.backdoorTimer = 0U;
            bootloader.state = BOOT_STATE_BOOTING;
            break;
        case BOOT_EVENT_ERASE_APP:
            bootloader.isAppValid = false;
            if (FlashManager_Erase(&bootloader.flashManager) == 0)
            {
                SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_ERASE_APP, NULL, 0);
                bootloader.state = BOOT_STATE_FLASHING;
            }
            else
            {
                bootloader.state = BOOT_STATE_IDLE;
            }
            break;
        default:
            break;
    }
}

static void Boot_HandleStateFlashing(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_FLASH_DATA:
        {
            /* Don't care about endianness, the data is sent in little endian */
            uint32_t address = *(uint32_t *)bootloader.flashBuffer;
            uint16_t size = *(uint16_t *)(bootloader.flashBuffer + 4);
            FlashManager_Write(&bootloader.flashManager, address, bootloader.flashBuffer + 6, size);
            SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_FLASH_DATA, NULL, 0);
            break;
        }
        case BOOT_EVENT_FLASH_CRC:
        {
            uint32_t crcValue = *(uint32_t *)bootloader.flashBuffer;
            uint32_t crcAddress = bootloader.flashManager.appEndAddress - 3U;
            FlashManager_Write(&bootloader.flashManager, crcAddress, (uint8_t *)&crcValue, sizeof(crcValue));
            SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_FLASH_CRC, NULL, 0);
            bootloader.state = BOOT_STATE_VERIFYING;
            break;
        }
        default:
            break;
    }
}

static void Boot_HandleStateVerifying(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_VALIDATE_APP:
            if (true == Boot_ValidateApp())
            {
                bootloader.isAppValid = true;
                bootloader.state = BOOT_STATE_IDLE;
                SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_VALIDATE_APP, NULL, 0);
            }
            else
            {
                bootloader.state = BOOT_STATE_ERASING;
            }
            break;
        default:
            break;
    }
}

static void Boot_CheckAndClearBootShared(uint32_t *bootFlags)
{
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        *bootFlags = 0U;
    }

    __HAL_RCC_CLEAR_RESET_FLAGS();
}


void Boot_Init(void)
{
    bootloader.backdoorTimer = 0U;
    bootloader.isAppValid = false;
    bootloader.noInitFlags = &bootFlags;
    bootloader.state = BOOT_STATE_IDLE;
    bootloader.backdoorTimerTimeout = BOOT_BACKDOOR_TIMEOUT;

    Boot_EventQueueInit(&bootloader.eventQueue);
    (void)SCP_Init(&bootloader.scpInstance);

    Boot_CheckAndClearBootShared(bootloader.noInitFlags);
    if (*bootloader.noInitFlags == BOOT_APP_RESET_FLAG)
    {
        /* Reset from application detected */
        Boot_AddEvent(BOOT_EVENT_START_DOWNLOAD);
        SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_START_DOWNLOAD, NULL, 0);
    }
}

void Boot_MainFunction(void)
{
    Boot_Event_T event;

    if (Boot_EventQueueDequeue(&bootloader.eventQueue, &event))
    {
        switch (bootloader.state)
        {
        case BOOT_STATE_IDLE:
            Boot_HandleStateIdle(event);
            break;
        case BOOT_STATE_BOOTING:
            Boot_HandleStateBooting(event);
            break;
        case BOOT_STATE_ERASING:
            Boot_HandleStateErasing(event);
            break;
        case BOOT_STATE_FLASHING:
            Boot_HandleStateFlashing(event);
            break;
        case BOOT_STATE_VERIFYING:
            Boot_HandleStateVerifying(event);
            break;
        default:
            break;
        }
    }
    else
    {
        /* No events to process, check SCP instance */
        SCP_Process(&bootloader);
    }
}

void Boot_AddEvent(Boot_Event_T event)
{
    if (!Boot_EventQueueEnqueue(&bootloader.eventQueue, event))
    {

    }
}
