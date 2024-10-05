#include "boot.h"

static Boot_EventQueue_T eventQueue;

void Boot_AddEvent(Boot_Event_T event)
{
    if (!Boot_EventQueueEnqueue(&eventQueue, event))
    {

    }
}
