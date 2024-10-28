/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "lf_signal_queue.h"
#include "cmsis_compiler.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define LF_Signal_EnterCritical() __disable_irq()
#define LF_Signal_ExitCritical() __enable_irq()

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static inline bool LF_SignalQueueIsFull(const LF_SignalQueue_T *const queue);
static inline bool LF_SignalQueueIsEmpty(const LF_SignalQueue_T *const queue);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Checks if the signal queue is full.
 *
 * @param[in] queue Pointer to the signal queue.
 *
 * @return
 * - true if the queue is full.
 * - false otherwise.
 */
static inline bool LF_SignalQueueIsFull(const LF_SignalQueue_T *const queue)
{
    return queue->count == LF_SIGNAL_QUEUE_SIZE;
}

/**
 * @brief Checks if the signal queue is empty.
 *
 * @param[in] queue Pointer to the signal queue.
 *
 * @return
 * - true if the queue is empty.
 * - false otherwise.
 */
static inline bool LF_SignalQueueIsEmpty(const LF_SignalQueue_T *const queue)
{
    return queue->count == 0U;
}

/**
 * @brief Initializes the signal queue.
 *
 * @param[in,out] queue Pointer to the signal queue to initialize.
 */
void LF_SignalQueue_Init(LF_SignalQueue_T *const queue)
{
    LF_Signal_EnterCritical();

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    LF_Signal_ExitCritical();
}


/**
 * @brief Enqueues a signal into the signal queue.
 *
 * @param[in,out] queue Pointer to the signal queue.
 * @param[in] sig The signal to be enqueued.
 *
 * @return
 * - true if the signal was successfully enqueued.
 * - false if the queue is full.
 */
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

/**
 * @brief Dequeues a signal from the signal queue.
 *
 * @param[in,out] queue Pointer to the signal queue.
 * @param[out] sig Pointer to the location where the dequeued signal will be stored.
 *
 * @return
 * - true if the signal was successfully dequeued.
 * - false if the queue is empty.
 */
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
