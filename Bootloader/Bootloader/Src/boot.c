/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "boot.h"
#include "scp.h"
#include "boot_commands.h"
#include "crc.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define BOOT_BACKDOOR_TIMEOUT   50U
#define BOOT_EXTENDED_TIMEOUT   30000U
#define BOOT_BLINK_TIME         1000U
#define BOOT_APP_RESET_FLAG     0xDEADBEEF
#define BOOT_SCP_BUFFER_SIZE    512U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
extern void Boot_JumpToApp(uint32_t address);

static void Boot_CheckAndClearBootShared(uint32_t *bootFlags);
static void Boot_HandleBlinking(void);
static bool Boot_ValidateApp(void);
static void Boot_HandleStateIdle(Boot_Event_T event);
static void Boot_HandleStateBooting(Boot_Event_T event);
static void Boot_HandleStateErasing(Boot_Event_T event);
static void Boot_HandleStateFlashing(Boot_Event_T event);
static void Boot_HandleStateVerifying(Boot_Event_T event);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/
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
            .sectorCount = 5U},
        .appStartAddress = 0x0800C000U,
        .appEndAddress = 0x0805FFFFU
        },
    .leds = {
        {LED1_GPIO_Port, LED1_Pin},
        {LED2_GPIO_Port, LED2_Pin},
        {LED3_GPIO_Port, LED3_Pin},
        {LED4_GPIO_Port, LED4_Pin},
        {LED5_GPIO_Port, LED5_Pin},
        {LED6_GPIO_Port, LED6_Pin},
        {LED7_GPIO_Port, LED7_Pin},
        {LED8_GPIO_Port, LED8_Pin},
        {LED9_GPIO_Port, LED9_Pin},
        {LED10_GPIO_Port, LED10_Pin},
        {LED11_GPIO_Port, LED11_Pin},
        {LED12_GPIO_Port, LED12_Pin},
    },
};

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Validates the integrity of the application firmware in flash memory.
 *
 * @return True if the application CRC matches the stored CRC; false otherwise.
 */
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

/**
 * @brief Handles the LED blinking functionality during bootloader operation.
 */
static void Boot_HandleBlinking(void)
{
    if (bootloader.blinkingEnabled)
    {
        bootloader.blinkTimer++;
        if (bootloader.blinkTimer >= BOOT_BLINK_TIME)
        {
            bootloader.blinkTimer = 0U;
            /* Toggle LEDs */
            for (uint32_t i = 0U; i < BOOT_LED_NUMBER; i++)
            {
                HAL_GPIO_TogglePin(bootloader.leds[i].port, bootloader.leds[i].pin);
            }
        }
    }
}

/**
 * @brief Handles the bootloader behavior when in the IDLE state.
 *
 * @param event The event to process in the IDLE state.
 */
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
            Boot_HandleBlinking();
            break;
        case BOOT_EVENT_START_DOWNLOAD:
            bootloader.blinkingEnabled = true;
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

/**
 * @brief Handles the bootloader behavior when in the BOOTING state.
 *
 * @param event The event to process in the BOOTING state.
 */
static void Boot_HandleStateBooting(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            Boot_HandleBlinking();
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

/**
 * @brief Handles the bootloader behavior when in the ERASING state.
 *
 * @param event The event to process in the ERASING state.
 */
static void Boot_HandleStateErasing(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            Boot_HandleBlinking();
            break;
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
                bootloader.blinkingEnabled = false;

                /* Turn off all LEDs */
                for (uint32_t i = 0U; i < BOOT_LED_NUMBER; i++)
                {
                    HAL_GPIO_WritePin(bootloader.leds[i].port, bootloader.leds[i].pin, GPIO_PIN_RESET);
                }
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

/**
 * @brief Handles the bootloader behavior when in the FLASHING state.
 *
 * @param event The event to process in the FLASHING state.
 */
static void Boot_HandleStateFlashing(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_FLASH_DATA:
        {
            /* Don't care about endianness, the data is sent in little endian */
            struct __attribute__((packed))
            {
                uint32_t address;
                uint16_t size;
                uint8_t progress;
                uint8_t data[];
            } *packet = bootloader.flashBuffer;

            FlashManager_Write(&bootloader.flashManager, packet->address, packet->data, packet->size);
            SCP_Transmit(&bootloader.scpInstance, BOOT_CMD_FLASH_DATA, NULL, 0);

            uint8_t numLedsOn = (packet->progress * BOOT_LED_NUMBER) / 100U;
            for (uint32_t i = 0U; i < BOOT_LED_NUMBER; i++)
            {
                if (i < numLedsOn)
                {
                    HAL_GPIO_WritePin(bootloader.leds[i].port, bootloader.leds[i].pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(bootloader.leds[i].port, bootloader.leds[i].pin, GPIO_PIN_RESET);
                }
            }
            break;
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

/**
 * @brief Handles the bootloader behavior when in the VERIFYING state.
 *
 * @param event The event to process in the VERIFYING state.
 */
static void Boot_HandleStateVerifying(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_VALIDATE_APP:
            if (true == Boot_ValidateApp())
            {
                bootloader.isAppValid = true;
                bootloader.state = BOOT_STATE_IDLE;
                bootloader.blinkingEnabled = true;
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

/**
 * @brief Checks and clears the boot shared flags in non-initialized shared memory.
 *
 * @param bootFlags Pointer to the boot flags variable in shared memory.
 */
static void Boot_CheckAndClearBootShared(uint32_t *bootFlags)
{
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        *bootFlags = 0U;
    }

    __HAL_RCC_CLEAR_RESET_FLAGS();
}

/**
 * @brief Initializes the bootloader.
 *
 * This function sets up the bootloader state, initializes peripherals,
 * and checks for any boot flags set by the application.
 */
void Boot_Init(void)
{
    bootloader.backdoorTimer = 0U;
    bootloader.isAppValid = false;
    bootloader.noInitFlags = &bootFlags;
    bootloader.state = BOOT_STATE_IDLE;
    bootloader.backdoorTimerTimeout = BOOT_BACKDOOR_TIMEOUT;
    bootloader.blinkTimer = 0;
    bootloader.blinkingEnabled = false;

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


/**
 * @brief Main function of the bootloader.
 *
 * This function should be called periodically to process events
 * and handle state transitions in the bootloader.
 */
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

/**
 * @brief Adds an event to the bootloader event queue.
 *
 * @param event The event to add to the queue.
 */
void Boot_AddEvent(Boot_Event_T event)
{
    if (!Boot_EventQueueEnqueue(&bootloader.eventQueue, event))
    {
        /* TODO: Handle event queue full scenario */
    }
}
