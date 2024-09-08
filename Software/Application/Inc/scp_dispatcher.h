#ifndef __SCP_DISPATCHER_H__
#define __SCP_DISPATCHER_H__

#include <stdint.h>

#define SCP_COMMAND_BUFFER_SIZE  64U
#define SCP_COMMAND_QUEUE_SIZE   5U

typedef struct
{
    char commands[SCP_COMMAND_QUEUE_SIZE][SCP_COMMAND_BUFFER_SIZE];
    uint8_t front;
    uint8_t rear;
    uint8_t count;
} SCP_DispatcherQueue_T;

#endif /* __SCP_DISPATCHER_H__ */
