#include "lf_main.h"
#include "lf_signal_queue.h"

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
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
}

bool LF_SignalQueueEnqueue(LF_SignalQueue_T *const queue, LF_Signal_T sig)
{
    if (LF_SignalQueueIsFull(queue))
    {
        return false;
    }

    queue->signals[queue->tail] = sig;
    queue->tail = (queue->tail + 1U) % LF_SIGNAL_QUEUE_SIZE;
    queue->count++;

    return true;
}

bool LF_SignalQueueDequeue(LF_SignalQueue_T *const queue, LF_Signal_T *sig)
{
    if (LF_SignalQueueIsEmpty(queue))
    {
        return false;
    }

    *sig = queue->signals[queue->head];
    queue->head = (queue->head + 1U) % LF_SIGNAL_QUEUE_SIZE;
    queue->count--;

    return true;
}
