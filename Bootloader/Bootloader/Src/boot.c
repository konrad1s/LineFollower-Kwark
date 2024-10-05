#include "boot.h"

#define BOOT_BACKDOOR_TIMEOUT 100U

typedef enum
{
    BOOT_STATE_IDLE,
    BOOT_STATE_BOOTING,
    BOOT_STATE_REFLASH_REQUESTED,
    BOOT_STATE_ERASING,
    BOOT_STATE_FLASHING,
    BOOT_STATE_VERIFYING,
    BOOT_STATE_INVALID_APP_DETECTED
} Boot_State_T;

static Boot_EventQueue_T eventQueue;
static Boot_State_T state = BOOT_STATE_IDLE;
static uint32_t bootBackdoorTimer = 0U;

static void Boot_HandleStateIdle(Boot_Event_T event)
{
    switch (event)
    {
        case BOOT_EVENT_TIMER_TICK:
            if (bootBackdoorTimer < BOOT_BACKDOOR_TIMEOUT)
            {
                bootBackdoorTimer++;
            }
            else
            {
                /* Boot backdoor timeout, try to boot the application */
                state = BOOT_STATE_BOOTING;
            }
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
        default:
            break;
    }
}

static void Boot_HandleStateFlashing(Boot_Event_T event)
{
    switch (event)
    {
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
    Boot_EventQueueInit(&eventQueue);

    bootBackdoorTimer = 0U;
}

void Boot_MainFunction(void)
{
    Boot_Event_T event;

    if (Boot_EventQueueDequeue(&eventQueue, &event))
    {
        switch (state)
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
}

void Boot_AddEvent(Boot_Event_T event)
{
    if (!Boot_EventQueueEnqueue(&eventQueue, event))
    {

    }
}
