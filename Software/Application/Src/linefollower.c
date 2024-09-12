#include "lf_main.h"

static void LF_Dispatch(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig);

static void Linefollower_DataUpdateCallback(void)
{
    LF_CalibrateSensors();
}

int LF_Init(LineFollower_T *const me)
{
    me->state = LF_IDLE;
    me->timer = 0U;
    me->isDataUpdated = false;
    me->nvmInstance.data = (uint8_t *)me->nvmBlock;

    LF_SignalQueue_Init(&me->signals);

    (void)NVM_Init(&me->nvmInstance);
    (void)NVM_Read(&me->nvmInstance);

    (void)PID_Init(&me->pidSensorInstance);

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        me->sensors[i].positionWeight = me->nvmBlock->sensorWeights[i];
        me->sensors[i].isActive = false;
    }
    (void)Sensors_Init(&hadc1, me->sensorLedsConfig, me->sensors, Linefollower_DataUpdateCallback);

    (void)TB6612Motor_Init(me->motorLeftConfig);
    (void)TB6612Motor_Init(me->motorRightConfig);
}

void LF_MainFunction(LineFollower_T *const me)
{
    LF_Signal_T sig;

    if (LF_SignalQueueDequeue(&me->signals, &sig))
    {
        switch (me->state)
        {
        case LF_IDLE:
            // LF_StateIdle(me, sig);
            break;
        case LF_CALIBRATION:
            // LF_StateCalibration(me, sig);
            break;
        case LF_RUN:
            // LF_StateRun(me, sig);
            break;
        case LF_ERROR:

            break;
        default:
            break;
        }
    }
}

void LF_SendSignal(LineFollower_T *const me, LF_Signal_T sig)
{
    if (!SignalQueue_Enqueue(&me->signals, sig))
    {
        /* TODO: Handle queue full */
    }
}
