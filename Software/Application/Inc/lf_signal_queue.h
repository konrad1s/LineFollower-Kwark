#ifndef __LF_SIGNAL_QUEUE_H__
#define __LF_SIGNAL_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define LF_SIGNAL_QUEUE_SIZE 5U

typedef enum 
{
    NO_SIG,
    START_SIG,
    STOP_SIG,
    CALIBRATE_SIG,
    DATA_UPDATED_SIG
} LF_Signal_T;

typedef struct
{
    LF_Signal_T signals[LF_SIGNAL_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} LF_SignalQueue_T;

void LF_SignalQueue_Init(LF_SignalQueue_T *const queue);
bool LF_SignalQueueEnqueue(LF_SignalQueue_T *const queue, LF_Signal_T sig);
bool LF_SignalQueueDequeue(LF_SignalQueue_T *const queue, LF_Signal_T *sig);

#endif /* __LF_SIGNAL_QUEUE_H__ */