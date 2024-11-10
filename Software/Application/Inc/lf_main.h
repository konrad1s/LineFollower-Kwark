#ifndef __LF_MAIN_H__
#define __LF_MAIN_H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "lf_signal_queue.h"
#include "nvm.h"
#include "scp.h"
#include "pid.h"
#include "sensors.h"
#include "tb6612_motor.h"
#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "encoder.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef enum 
{
    LF_IDLE,
    LF_CALIBRATION,
    LF_RUN,
    LF_ERROR
} LFState_T;

typedef struct __attribute__((packed))
{
    uint16_t sensorsValues[SENSORS_NUMBER];
    float sensorError;
    float motorLeftVelocity;
    float motorRightVelocity;
    bool isSpeedReduced;
} Lf_DebugData_T;

typedef struct
{
    uint32_t tick;
    LF_Signal_T associatedTimeoutSig;
} LF_Timer_T;

typedef struct
{
    LFState_T state;
    uint32_t *bootFlags;
    uint32_t prevCycleCount;
    bool isDebugMode;
    volatile uint32_t *const cycleCountReg;
    TIM_HandleTypeDef *const debugModeTimer;
    LF_SignalQueue_T signals;
    Lf_DebugData_T debugData;
    LF_Timer_T timers[LF_TIMER_NB];

    Nvm_Instance_T nvmInstance;
    NVM_Layout_T *const nvmBlock;
    SCP_Instance_T scpInstance;
    PID_Instance_T pidSensorInstance;
    PID_Instance_T pidEncoderLeftInstance;
    PID_Instance_T pidEncoderRightInstance;
    Encoder_Instance_T encoderLeft;
    Encoder_Instance_T encoderRight;
    Sensors_Instance_T sensorsInstance;

    const TB6612MotorDriver_T *const motorLeft;
    const TB6612MotorDriver_T *const motorRight;
} LineFollower_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
int LF_Init(LineFollower_T *const me);
void LF_MainFunction(LineFollower_T *const me);
void LF_SendSignal(LineFollower_T *const me, LF_Signal_T sig);
void LF_DebugModeTimerCallback(void *context);

#endif /* __LF_MAIN_H__ */
