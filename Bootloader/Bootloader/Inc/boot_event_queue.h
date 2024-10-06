#ifndef __BOOT_EVENT_QUEUE_H__
#define __BOOT_EVENT_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define BOOT_EVENT_QUEUE_SIZE 10U

typedef enum 
{
    BOOT_EVENT_START_DOWNLOAD,
    BOOT_EVENT_ERASE_APP,
    BOOT_EVENT_FLASH_DATA,
    BOOT_EVENT_FLASH_MAC,
    BOOT_EVENT_VALIDATE_APP,
    BOOT_EVENT_JUMP_TO_APP,
    BOOT_EVENT_TIMER_TICK,
    BOOT_EVENT_FRAME_WAIT_TIMEOUT,
} Boot_Event_T;

typedef struct
{
    Boot_Event_T events[BOOT_EVENT_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} Boot_EventQueue_T;

void Boot_EventQueueInit(Boot_EventQueue_T *const queue);
bool Boot_EventQueueEnqueue(Boot_EventQueue_T *const queue, Boot_Event_T event);
bool Boot_EventQueueDequeue(Boot_EventQueue_T *const queue, Boot_Event_T *event);

#endif /* __BOOT_EVENT_QUEUE_H__ */
