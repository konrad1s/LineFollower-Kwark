#ifndef __SCP_DISPATCHER_H__
#define __SCP_DISPATCHER_H__

#include <stdint.h>

#define SCP_GLOBAL_BUFFER_SIZE 256U

typedef struct
{
    uint8_t buffer[SCP_GLOBAL_BUFFER_SIZE];
    uint16_t front;
    uint16_t rear;
    uint16_t count;
} SCP_DispatcherQueue_T;

#endif /* __SCP_DISPATCHER_H__ */
