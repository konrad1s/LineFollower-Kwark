#ifndef __LF_MAIN_H__
#define __LF_MAIN_H__

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

typedef enum 
{
    LF_IDLE,
    LF_CALIBRATION,
    LF_RUN,
    LF_ERROR
} LFState_T;

typedef struct
{
    LFState_T state;
    uint32_t timer;
    bool isDataUpdated;
    LF_SignalQueue_T signals;

    Nvm_Instance_T nvmInstance;
    NVM_Layout_T *const nvmBlock;
    SCP_Instance_T scpInstance;
    PID_Instance_T pidSensorInstance;
    Sensor_Instance_T sensors[SENSORS_NUMBER];

    const Sensor_Led_T *sensorLedsConfig;
    const TB6612MotorDriver_T *motorLeftConfig;
    const TB6612MotorDriver_T *motorRightConfig;
} LineFollower_T;

int LF_Init(LineFollower_T *const me);
void LF_MainFunction(LineFollower_T *const me);
void LF_SendSignal(LineFollower_T *const me, LF_Signal_T sig);

#endif /* __LF_MAIN_H__ */