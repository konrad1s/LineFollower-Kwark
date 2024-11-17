/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "boot_event_queue.h"
#include "cmsis_compiler.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define Boot_Event_EnterCritical() __disable_irq()
#define Boot_Event_ExitCritical() __enable_irq()

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Checks if the event queue is full.
 *
 * @param[in] queue Pointer to the event queue.
 *
 * @return
 * - true if the queue is full.
 * - false otherwise.
 */
static inline bool Boot_EventQueueIsFull(const Boot_EventQueue_T *const queue)
{
    return queue->count == BOOT_EVENT_QUEUE_SIZE;
}

/**
 * @brief Checks if the event queue is empty.
 *
 * @param[in] queue Pointer to the event queue.
 *
 * @return
 * - true if the queue is empty.
 * - false otherwise.
 */
static inline bool Boot_EventQueueIsEmpty(const Boot_EventQueue_T *const queue)
{
    return queue->count == 0U;
}

/**
 * @brief Initializes the event queue.
 *
 * @param[in,out] queue Pointer to the event queue to initialize.
 */
void Boot_EventQueueInit(Boot_EventQueue_T *const queue)
{
    Boot_Event_EnterCritical();

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    Boot_Event_ExitCritical();
}


/**
 * @brief Enqueues a signal into the event queue.
 *
 * @param[in,out] queue Pointer to the event queue.
 * @param[in] event The signal to be enqueued.
 *
 * @return
 * - true if the signal was successfully enqueued.
 * - false if the queue is full.
 */
bool Boot_EventQueueEnqueue(Boot_EventQueue_T *const queue, Boot_Event_T event)
{
    Boot_Event_EnterCritical();

    if (Boot_EventQueueIsFull(queue))
    {
        Boot_Event_ExitCritical();
        return false;
    }

    queue->events[queue->tail] = event;
    queue->tail = (queue->tail + 1U) % BOOT_EVENT_QUEUE_SIZE;
    queue->count++;

    Boot_Event_ExitCritical();

    return true;
}

/**
 * @brief Dequeues a signal from the event queue.
 *
 * @param[in,out] queue Pointer to the event queue.
 * @param[out] event Pointer to the location where the dequeued signal will be stored.
 *
 * @return
 * - true if the signal was successfully dequeued.
 * - false if the queue is empty.
 */
bool Boot_EventQueueDequeue(Boot_EventQueue_T *const queue, Boot_Event_T *event)
{
    Boot_Event_EnterCritical();

    if (Boot_EventQueueIsEmpty(queue))
    {
        Boot_Event_ExitCritical();
        return false;
    }

    *event = queue->events[queue->head];
    queue->head = (queue->head + 1U) % BOOT_EVENT_QUEUE_SIZE;
    queue->count--;

    Boot_Event_ExitCritical();

    return true;
}
