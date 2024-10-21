#include "lf_main.h"
#include "lf_calibrate.h"
#include <string.h>

static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig);

__attribute__((section(".noinit_shared"))) static uint32_t bootloaderFlags;

static void LF_SendDebugData(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    Sensors_GetRawData(me->debugData.sensorsValues);
    me->debugData.motorLeftVelocity = me->encoderLeft.velocity;
    me->debugData.motorRightVelocity = me->encoderRight.velocity;

    SCP_Transmit(&me->scpInstance, LF_CMD_SEND_DEBUG_DATA, &me->debugData, sizeof(me->debugData));
}

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
        me->debugData.sensorError = Sensors_CalculateError(&me->nvmBlock->sensors);
        Sensors_UpdateLeds();
        break;
    case LF_SIG_SEND_DEBUG_DATA:
        LF_SendDebugData(NULL, me);
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
        LF_UpdateCalibrationData(me);
        break;
    case LF_SIG_TIMER_TICK:
        {
            LF_CalibrationStatus_T status = LF_UpdateCalibrationTimer(me);

            if (LF_CALIBRATION_COMPLETE == status)
            {
                SCP_Transmit(&me->scpInstance, LF_CMD_CALIBRATE, NULL, 0);
                me->state = LF_IDLE;
            }
            else if (LF_CALIBRATION_ERROR == status)
            {
                SCP_Transmit(&me->scpInstance, LF_CMD_CALIBRATE, NULL, 0);
                me->state = LF_IDLE;
            }
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
        float dt = 20.0;
        if (true == Sensors_AnySensorDetectedLine())
        {
            me->noLineDetectedCounter = 0U;
        }
        me->debugData.sensorError = Sensors_CalculateError(&me->nvmBlock->sensors);
        float pidSensorOutput = PID_Update(&me->pidSensorInstance, me->debugData.sensorError, dt);
        float targetSpeedLeft = me->nvmBlock->targetSpeed - pidSensorOutput;
        float targetSpeedRight = me->nvmBlock->targetSpeed + pidSensorOutput;

        Encoder_Update(&me->encoderLeft, dt);
        Encoder_Update(&me->encoderRight, dt);

        me->pidEncoderLeftInstance.setpoint = targetSpeedLeft;
        me->pidEncoderRightInstance.setpoint = targetSpeedRight;

        float pidEncoderLeftOutput = PID_Update(&me->pidEncoderLeftInstance, me->encoderLeft.velocity, dt);
        float pidEncoderRightOutput = PID_Update(&me->pidEncoderRightInstance, me->encoderRight.velocity, dt);

        TB6612Motor_SetSpeed(me->motorLeftConfig, (uint16_t)pidEncoderLeftOutput);
        TB6612Motor_SetSpeed(me->motorRightConfig, (uint16_t)pidEncoderRightOutput);

        Sensors_UpdateLeds();
        break;
    case LF_SIG_SEND_DEBUG_DATA:
        LF_SendDebugData(NULL, me);
        break;
    case LF_SIG_TIMER_TICK:
        me->noLineDetectedCounter++;
        if (me->noLineDetectedCounter >= me->nvmBlock->noLineDetectedTimeout)
        {
            TB6612Motor_Stop(me->motorLeftConfig);
            TB6612Motor_Stop(me->motorRightConfig);
            me->noLineDetectedCounter = 0U;
            me->state = LF_IDLE;
        }
        break;
    default:
        break;
    }
}

int LF_Init(LineFollower_T *const me)
{
    me->state = LF_IDLE;
    me->isDebugMode = false;
    me->timer = 0U;
    me->nvmInstance.data = (uint8_t *)me->nvmBlock;
    me->bootFlags = &bootloaderFlags;
    me->noLineDetectedCounter = 0U;

    LF_SignalQueue_Init(&me->signals);
    memset(&me->debugData, 0, sizeof(me->debugData));

    (void)NVM_Init(&me->nvmInstance);
    (void)NVM_Read(&me->nvmInstance);

    (void)PID_Init(&me->pidSensorInstance);
    (void)PID_Init(&me->pidEncoderLeftInstance);
    (void)PID_Init(&me->pidEncoderRightInstance);
    (void)SCP_Init(&me->scpInstance);
    (void)Encoder_Init(&me->encoderLeft);
    (void)Encoder_Init(&me->encoderRight);

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        me->sensors[i].positionWeight = me->nvmBlock->sensors.weights[i];
        me->sensors[i].isActive = false;
    }
    (void)Sensors_Init(&hadc1, me->sensorLedsConfig, me->sensors, Linefollower_DataUpdateCallback, me);
    Sensors_SetThresholds(me->nvmBlock->sensors.thresholds);

    (void)TB6612Motor_Init(me->motorLeftConfig);
    (void)TB6612Motor_Init(me->motorRightConfig);

    return 0;
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

void LF_DebugModeTimerCallback(void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    LF_SendSignal(me, LF_SIG_SEND_DEBUG_DATA);
}
