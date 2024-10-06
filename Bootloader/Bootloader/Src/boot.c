#include "boot.h"
#include "scp.h"
#include "boot_commands.h"

#define BOOT_BACKDOOR_TIMEOUT 50000U
#define BOOT_SCP_BUFFER_SIZE 4096U

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
            .startSector = FLASH_SECTOR_1,
            .sectorCount = 1U}
        },
};

static void Boot_HandleStateIdle(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            if (bootloader.backdoorTimer < BOOT_BACKDOOR_TIMEOUT)
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
            bootloader.state = BOOT_STATE_ERASING;
            break;
        default:
            break;
    }
}

static void Boot_HandleStateBooting(Boot_Event_T event)
{
    switch (event)
    {
        default:
            break;
    }
}

static void Boot_HandleStateErasing(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_ERASE_APP:
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
            FlashManager_Write(&bootloader.flashManager, address, bootloader.flashBuffer, size);
            SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_FLASH_DATA, NULL, 0);
            break;
        }
        case BOOT_EVENT_FLASH_MAC:
            SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_FLASH_MAC, NULL, 0);
            bootloader.state = BOOT_EVENT_VALIDATE_APP;
            break;
        default:
            break;
    }
}

static void Boot_HandleStateVerifying(Boot_Event_T event)
{
    switch (event)
    {
        default:
            break;
    }
}

static void Boot_HandleStateInvalidAppDetected(Boot_Event_T event)
{
    switch (event)
    {
        default:
            break;
    }
}

void Boot_Init(void)
{
    Boot_EventQueueInit(&bootloader.eventQueue);
    (void)SCP_Init(&bootloader.scpInstance);

    bootloader.backdoorTimer = 0U;
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
        case BOOT_STATE_INVALID_APP_DETECTED:
            Boot_HandleStateInvalidAppDetected(event);
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
