#include "lf_main.h"
#include "lf_calibrate.h"

static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig);

static void Linefollower_DataUpdateCallback(void *data)
{
    LineFollower_T *const me = (LineFollower_T *const )data;

    LF_SendSignal(me, LF_SIG_ADC_DATA_UPDATED);
}

static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_START:
        me->state = LF_RUN;
        break;
    case LF_SIG_CALIBRATE:
        LF_StartCalibration(me);
        me->state = LF_CALIBRATION;
        break;
    case LF_SIG_ADC_DATA_UPDATED:
        Sensors_UpdateLeds();
        break;
    default:
        break;
    }
}

static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_ADC_DATA_UPDATED:
        LF_CalibrationStatus_T status = LF_CalibrateSensors(me);

        if (LF_CALIBRATION_COMPLETE == status)
        {
            me->state = LF_IDLE;
        }
        break;
    default:
        break;
    }
}

static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_STOP:
        TB6612Motor_Stop(me->motorLeftConfig);
        TB6612Motor_Stop(me->motorRightConfig);
        me->state = LF_IDLE;
        break;
    case LF_SIG_ADC_DATA_UPDATED:
        float error = Sensors_CalculateError(me->nvmBlock);
        int16_t output = PID_Update(&me->pidSensorInstance, error, 1.0);
        TB6612Motor_SetSpeed(me->motorLeftConfig, 100U - output);
        TB6612Motor_SetSpeed(me->motorRightConfig, 100U + output);
        Sensors_UpdateLeds();
        break;
    default:
        break;
    }
}

int LF_Init(LineFollower_T *const me)
{
    me->state = LF_IDLE;
    me->timer = 0U;
    me->isDebugMode = false;
    me->nvmInstance.data = (uint8_t *)me->nvmBlock;

    LF_SignalQueue_Init(&me->signals);

    (void)NVM_Init(&me->nvmInstance);
    (void)NVM_Read(&me->nvmInstance);

    (void)PID_Init(&me->pidSensorInstance);
    (void)SCP_Init(&me->scpInstance);

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        me->sensors[i].positionWeight = me->nvmBlock->sensors.weights[i];
        me->sensors[i].isActive = false;
    }
    (void)Sensors_Init(&hadc1, me->sensorLedsConfig, me->sensors, Linefollower_DataUpdateCallback, me);
    Sensors_SetThresholds(me->nvmBlock->sensors.thresholds);

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
            LF_StateIdle(me, sig);
            break;
        case LF_CALIBRATION:
            LF_StateCalibration(me, sig);
            break;
        case LF_RUN:
            LF_StateRun(me, sig);
            break;
        case LF_ERROR:

            break;
        default:
            break;
        }
    }
    else
    {
        /* No signals to process, so let's process the communication protocol */
        SCP_Process(me);
    }
}

void LF_SendSignal(LineFollower_T *const me, LF_Signal_T sig)
{
    if (!LF_SignalQueueEnqueue(&me->signals, sig))
    {
        /* TODO: Handle queue full */
    }
}
