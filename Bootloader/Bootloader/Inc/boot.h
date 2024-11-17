#ifndef __BOOT_H__
#define __BOOT_H__
/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "boot_event_queue.h"
#include "scp.h"
#include "flash_manager.h"
#include "gpio.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define BOOT_FLASH_BUFFER_SIZE 260U
#define BOOT_LED_NUMBER        12U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef enum
{
    BOOT_STATE_IDLE,
    BOOT_STATE_BOOTING,
    BOOT_STATE_ERASING,
    BOOT_STATE_FLASHING,
    BOOT_STATE_VERIFYING
} Boot_State_T;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} Boot_Led_T;

typedef struct
{
    Boot_State_T state;
    uint32_t backdoorTimer;
    uint32_t backdoorTimerTimeout;
    uint32_t blinkTimer;
    uint32_t *noInitFlags;
    uint8_t flashBuffer[BOOT_FLASH_BUFFER_SIZE];
    bool isAppValid;
    bool blinkingEnabled;
    Boot_Led_T leds[BOOT_LED_NUMBER];
    Boot_EventQueue_T eventQueue;

    SCP_Instance_T scpInstance;
    Flash_Manager_T flashManager;
} Bootloader_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
void Boot_Init(void);
void Boot_MainFunction(void);
void Boot_AddEvent(Boot_Event_T event);

#endif /* __BOOT_H__ */
