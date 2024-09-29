#include "lf_signal_queue.h"

#define LF_Signal_EnterCritical() __disable_irq()
#define LF_Signal_ExitCritical() __enable_irq()

static inline bool LF_SignalQueueIsFull(const LF_SignalQueue_T *const queue)
{
    return queue->count == LF_SIGNAL_QUEUE_SIZE;
}

static inline bool LF_SignalQueueIsEmpty(const LF_SignalQueue_T *const queue)
{
    return queue->count == 0U;
}

void LF_SignalQueue_Init(LF_SignalQueue_T *const queue)
{
    LF_Signal_EnterCritical();

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    LF_Signal_ExitCritical();
}

bool LF_SignalQueueEnqueue(LF_SignalQueue_T *const queue, LF_Signal_T sig)
{
    LF_Signal_EnterCritical();

    if (LF_SignalQueueIsFull(queue))
    {
        LF_Signal_ExitCritical();
        return false;
    }

    queue->signals[queue->tail] = sig;
    queue->tail = (queue->tail + 1U) % LF_SIGNAL_QUEUE_SIZE;
    queue->count++;

    LF_Signal_ExitCritical();

    return true;
}

bool LF_SignalQueueDequeue(LF_SignalQueue_T *const queue, LF_Signal_T *sig)
{
    LF_Signal_EnterCritical();

    if (LF_SignalQueueIsEmpty(queue))
    {
        LF_Signal_ExitCritical();
        return false;
    }

    *sig = queue->signals[queue->head];
    queue->head = (queue->head + 1U) % LF_SIGNAL_QUEUE_SIZE;
    queue->count--;

    LF_Signal_ExitCritical();

    return true;
}
