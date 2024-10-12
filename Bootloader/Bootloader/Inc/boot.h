#ifndef __BOOT_H__
#define __BOOT_H__

#include <stdint.h>
#include "boot_event_queue.h"
#include "scp.h"
#include "flash_manager.h"

#define BOOT_FLASH_BUFFER_SIZE 2048U

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
    Boot_State_T state;
    uint32_t backdoorTimer;
    uint8_t flashBuffer[BOOT_FLASH_BUFFER_SIZE];
    Boot_EventQueue_T eventQueue;
    bool isAppValid;

    SCP_Instance_T scpInstance;
    Flash_Manager_T flashManager;
} Bootloader_T;

void Boot_Init(void);
void Boot_MainFunction(void);
void Boot_AddEvent(Boot_Event_T event);

#endif /* __BOOT_H__ */
