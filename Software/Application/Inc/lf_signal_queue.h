#ifndef __LF_SIGNAL_QUEUE_H__
#define __LF_SIGNAL_QUEUE_H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define LF_SIGNAL_QUEUE_SIZE 5U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef enum 
{
    LF_SIG_START,
    LF_SIG_STOP,
    LF_SIG_CALIBRATE,
    LF_SIG_ADC_DATA_UPDATED,
    LF_SIG_SEND_DEBUG_DATA,
    LF_SIG_TIMER_TICK
} LF_Signal_T;

typedef struct
{
    LF_Signal_T signals[LF_SIGNAL_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} LF_SignalQueue_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
void LF_SignalQueue_Init(LF_SignalQueue_T *const queue);
bool LF_SignalQueueEnqueue(LF_SignalQueue_T *const queue, LF_Signal_T sig);
bool LF_SignalQueueDequeue(LF_SignalQueue_T *const queue, LF_Signal_T *sig);

#endif /* __LF_SIGNAL_QUEUE_H__ */
