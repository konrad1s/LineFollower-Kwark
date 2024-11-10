/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "lf_main.h"
#include "lf_calibrate.h"
#include <string.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define LF_InitHandleFailure(status)    LF_LogError(__FILE__, __LINE__, status)
#define LF_IsTimerOn(timer)             ((timer).tick != UINT32_MAX)
#define LF_StartTimer(timer)            ((timer).tick = 0U)
#define LF_StopTimer(timer)             ((timer).tick = UINT32_MAX)
#define LF_RefreshTimer(timer)          ((timer).tick = 0U)
#define LF_TimerTick(timer)             ((timer).tick++)
#define LF_PID_UPDATE_INTERVAL_MS       5.0f
#define LF_MAX_MOTOR_SPEED              999U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef enum
{
    LF_SUCCESS = 0,
    LF_ERROR_NVM_INIT = -1,
    LF_ERROR_NVM_READ = -2,
    LF_ERROR_PID_INIT = -3,
    LF_ERROR_ENCODER_INIT = -4,
    LF_ERROR_SENSOR_INIT = -5,
    LF_ERROR_MOTOR_INIT = -6,
    LF_ERROR_COMM_INIT = -7
} LF_ErrorCode_T;

typedef LF_ErrorCode_T (*LF_ComponentInitFunc)(LineFollower_T *const me);

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static void LF_InitState(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitComponents(LineFollower_T *const me);

/* Component Initialization Functions */
static LF_ErrorCode_T LF_ComponentInit(LF_ComponentInitFunc initFunc, LineFollower_T *const me, const char *componentName);
static LF_ErrorCode_T LF_InitNVM(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitPID(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitEncoders(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitSensors(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitMotors(LineFollower_T *const me);
static LF_ErrorCode_T LF_InitCommunication(LineFollower_T *const me);

/* State Handling Functions */
static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig);
static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig);

/* LF_StateRun Helper Functions */
static void LF_HandleStopSignal(LineFollower_T *const me);
static void LF_HandleADCDataUpdated(LineFollower_T *const me);
static void LF_HandleTimerTick(LineFollower_T *const me);

/* Other Functions */
static void LF_SendDebugData(const SCP_Packet *const packet, void *context);
static void LF_DataUpdateCallback(void *data);
static uint16_t LF_ClampMotorSpeed(float speed);
static void LF_LogError(const char *file, int line, LF_ErrorCode_T errorCode);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/
__attribute__((section(".noinit_shared"))) static uint32_t bootloaderFlags;

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Initializes the LineFollower instance.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @return 0 on success, non-zero error code on failure.
 */
int LF_Init(LineFollower_T *const me)
{
    LF_InitState(me);
    LF_ErrorCode_T status = LF_InitComponents(me);
    return status;
}

/**
 * @brief Initializes the basic state and variables.
 *
 * @param[in] me Pointer to the LineFollower instance.
 */
static void LF_InitState(LineFollower_T *const me)
{
    me->state = LF_IDLE;
    me->isDebugMode = false;
    me->bootFlags = &bootloaderFlags;
    me->prevCycleCount = 0U;

    for (LF_TimetId_T timer = 0; timer < LF_TIMER_NB; timer++)
    {
        LF_StopTimer(me->timers[timer]);
        me->timers[timer].associatedTimeoutSig = LF_SIG_INVALID;
    }
    
    me->timers[LF_TIMER_NO_LINE_DETECTED].associatedTimeoutSig = LF_SIG_STOP;
    me->timers[LF_TIMER_CALIBRATION].associatedTimeoutSig = LF_SIG_CALIBRATION_COMPLETE;

    LF_SignalQueue_Init(&me->signals);
    memset(&me->debugData, 0, sizeof(me->debugData));
}

/**
 * @brief Initializes all components.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @return 0 on success, non-zero error code on failure.
 */
static LF_ErrorCode_T LF_InitComponents(LineFollower_T *const me)
{
    LF_ErrorCode_T status = LF_SUCCESS;

    if ((status = LF_ComponentInit(LF_InitNVM, me, "NVM")) != LF_SUCCESS)
    {
        return status;
    }

    if ((status = LF_ComponentInit(LF_InitPID, me, "PID Controllers")) != LF_SUCCESS)
    {
        return status;
    }

    if ((status = LF_ComponentInit(LF_InitEncoders, me, "Encoders")) != LF_SUCCESS)
    {
        return status;
    }

    if ((status = LF_ComponentInit(LF_InitSensors, me, "Sensors")) != LF_SUCCESS)
    {
        return status;
    }

    if ((status = LF_ComponentInit(LF_InitMotors, me, "Motors")) != LF_SUCCESS)
    {
        return status;
    }

    if ((status = LF_ComponentInit(LF_InitCommunication, me, "Communication")) != LF_SUCCESS)
    {
        return status;
    }

    return LF_SUCCESS;
}

/**
 * @brief Generic component initialization helper function.
 *
 * @param[in] initFunc Initialization function for the component.
 * @param[in] me Pointer to the LineFollower instance.
 * @param[in] componentName Name of the component (for logging).
 * @return 0 on success, non-zero error code on failure.
 */
static LF_ErrorCode_T LF_ComponentInit(LF_ComponentInitFunc initFunc, LineFollower_T *const me, const char *componentName)
{
    LF_ErrorCode_T status = initFunc(me);
    if (status != LF_SUCCESS)
    {
        LF_InitHandleFailure(status);
        return status;
    }
    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitNVM(LineFollower_T *const me)
{
    me->nvmInstance.data = (uint8_t *)me->nvmBlock;

    if (NVM_Init(&me->nvmInstance) != 0)
    {
        return LF_ERROR_NVM_INIT;
    }

    if (NVM_Read(&me->nvmInstance) != 0)
    {
        return LF_ERROR_NVM_READ;
    }

    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitPID(LineFollower_T *const me)
{
    if (PID_Init(&me->pidSensorInstance) != 0)
    {
        return LF_ERROR_PID_INIT;
    }

    if (PID_Init(&me->pidEncoderLeftInstance) != 0)
    {
        return LF_ERROR_PID_INIT;
    }

    if (PID_Init(&me->pidEncoderRightInstance) != 0)
    {
        return LF_ERROR_PID_INIT;
    }

    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitEncoders(LineFollower_T *const me)
{
    if (Encoder_Init(&me->encoderLeft) != 0)
    {
        return LF_ERROR_ENCODER_INIT;
    }

    if (Encoder_Init(&me->encoderRight) != 0)
    {
        return LF_ERROR_ENCODER_INIT;
    }

    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitSensors(LineFollower_T *const me)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        me->sensorsInstance.sensors[i].positionWeight = me->nvmBlock->sensors.weights[i];
        me->sensorsInstance.sensors[i].isActive = false;
    }

    if (Sensors_Init(&me->sensorsInstance, LF_DataUpdateCallback, me) != 0)
    {
        return LF_ERROR_SENSOR_INIT;
    }

    Sensors_SetThresholds(&me->sensorsInstance, me->nvmBlock->sensors.thresholds);

    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitMotors(LineFollower_T *const me)
{
    if (TB6612Motor_Init(me->motorLeft) != 0)
    {
        return LF_ERROR_MOTOR_INIT;
    }

    if (TB6612Motor_Init(me->motorRight) != 0)
    {
        return LF_ERROR_MOTOR_INIT;
    }

    return LF_SUCCESS;
}

static LF_ErrorCode_T LF_InitCommunication(LineFollower_T *const me)
{
    if (SCP_Init(&me->scpInstance) != 0)
    {
        return LF_ERROR_COMM_INIT;
    }

    return LF_SUCCESS;
}

/**
 * @brief Handles the LF_RUN state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @param[in] sig Signal received.
 */
static void LF_StateRun(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_STOP:
        LF_HandleStopSignal(me);
        break;

    case LF_SIG_ADC_DATA_UPDATED:
        LF_HandleADCDataUpdated(me);
        break;

    case LF_SIG_SEND_DEBUG_DATA:
        LF_SendDebugData(NULL, me);
        break;

    case LF_SIG_TIMER_TICK:
        LF_HandleTimerTick(me);
        break;

    default:
        break;
    }
}

/**
 * @brief Handles the LF_SIG_STOP signal in the LF_RUN state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 */
static void LF_HandleStopSignal(LineFollower_T *const me)
{
    TB6612Motor_Stop(me->motorLeft);
    TB6612Motor_Stop(me->motorRight);

    for (LF_TimetId_T timer = 0; timer < LF_TIMER_NB; timer++)
    {
        LF_StopTimer(me->timers[timer]);
    }

    me->prevCycleCount = 0U;
    me->state = LF_IDLE;
}

/**
 * @brief Handles the LF_SIG_ADC_DATA_UPDATED signal in the LF_RUN state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 */
static void LF_HandleADCDataUpdated(LineFollower_T *const me)
{
    uint32_t currCycleCount = *(me->cycleCountReg);

    if (me->prevCycleCount == 0U)
    {
        me->prevCycleCount = currCycleCount;
        return;
    }

    uint32_t cycleDiff = currCycleCount - me->prevCycleCount;
    me->prevCycleCount = currCycleCount;
    float dt = ((float)cycleDiff) / SystemCoreClock * 1000.0f;
    bool isSpeedReduced = false;

    if (me->sensorsInstance.anySensorDetectedLine)
    {
        LF_RefreshTimer(me->timers[LF_TIMER_NO_LINE_DETECTED]);
    }

    if (me->sensorsInstance.rightAngleDetected)
    {
        LF_StartTimer(me->timers[LF_TIMER_REDUCED_SPEED]);
        LF_StopTimer(me->timers[LF_TIMER_SENSORS_STABILIZE]);
        isSpeedReduced = true;
    }
    else if (LF_IsTimerOn(me->timers[LF_TIMER_SENSORS_STABILIZE]))
    {
        if (!me->sensorsInstance.stabilizeDetected)
        {
            LF_RefreshTimer(me->timers[LF_TIMER_SENSORS_STABILIZE]);
        }
        isSpeedReduced = true;
    }

    float targetSpeedLeft = me->nvmBlock->targetSpeed;
    float targetSpeedRight = me->nvmBlock->targetSpeed;

    if (isSpeedReduced)
    {
        targetSpeedLeft *= 0.85f;
        targetSpeedRight *= 0.85f;
    }

    me->debugData.sensorError = Sensors_CalculateError(&me->sensorsInstance, &me->nvmBlock->sensors);
    float pidSensorOutput = PID_Update(&me->pidSensorInstance, me->debugData.sensorError, dt);
    targetSpeedLeft -= pidSensorOutput;
    targetSpeedRight += pidSensorOutput;

    Encoder_Update(&me->encoderLeft, dt);
    Encoder_Update(&me->encoderRight, dt);

    me->pidEncoderLeftInstance.setpoint = targetSpeedLeft;
    me->pidEncoderRightInstance.setpoint = targetSpeedRight;

    float pidEncoderLeftOutput = PID_Update(&me->pidEncoderLeftInstance, me->encoderLeft.velocity, dt);
    float pidEncoderRightOutput = PID_Update(&me->pidEncoderRightInstance, me->encoderRight.velocity, dt);

    uint16_t leftMotorSpeed = LF_ClampMotorSpeed(pidEncoderLeftOutput);
    uint16_t rightMotorSpeed = LF_ClampMotorSpeed(pidEncoderRightOutput);

    TB6612Motor_ChangeDirection(me->motorLeft, MOTOR_FORWARD);
    TB6612Motor_ChangeDirection(me->motorRight, MOTOR_FORWARD);
    TB6612Motor_SetSpeed(me->motorLeft, leftMotorSpeed);
    TB6612Motor_SetSpeed(me->motorRight, rightMotorSpeed);

    Sensors_UpdateLeds(&me->sensorsInstance);
}

/**
 * @brief Handles the LF_SIG_TIMER_TICK signal in the LF_RUN state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 */
static void LF_HandleTimerTick(LineFollower_T *const me)
{
    for (LF_TimetId_T timer = 0; timer < LF_TIMER_NB; timer++)
    {
        if (LF_IsTimerOn(me->timers[timer]))
        {
            me->timers[timer].tick++;
            if (me->timers[timer].tick >= me->nvmBlock->timerTimeout[timer])
            {
                LF_StopTimer(me->timers[timer]);
                if (me->timers[timer].associatedTimeoutSig != LF_SIG_INVALID)
                {
                    LF_SendSignal(me, me->timers[timer].associatedTimeoutSig);
                }
            }
        }
    }
}

/**
 * @brief Handles the LF_IDLE state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @param[in] sig Signal received.
 */
static void LF_StateIdle(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_START:
        (void)LF_InitPID(me);
        (void)LF_InitEncoders(me);
        me->state = LF_RUN;
        break;
    case LF_SIG_CALIBRATE:
        LF_StartCalibration(me);
        LF_StartTimer(me->timers[LF_TIMER_CALIBRATION]);
        me->state = LF_CALIBRATION;
        break;
    case LF_SIG_ADC_DATA_UPDATED:
        me->debugData.sensorError = Sensors_CalculateError(&me->sensorsInstance, &me->nvmBlock->sensors);
        Sensors_UpdateLeds(&me->sensorsInstance);
        break;
    case LF_SIG_SEND_DEBUG_DATA:
        LF_SendDebugData(NULL, me);
        break;
    case LF_SIG_TIMER_TICK:
        LF_HandleTimerTick(me);
    default:
        break;
    }
}

/**
 * @brief Handles the LF_CALIBRATION state.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @param[in] sig Signal received.
 */
static void LF_StateCalibration(LineFollower_T *const me, LF_Signal_T sig)
{
    switch (sig)
    {
    case LF_SIG_ADC_DATA_UPDATED:
        LF_UpdateCalibrationData(me);
        break;

    case LF_SIG_CALIBRATION_COMPLETE:
        LF_StopTimer(me->timers[LF_TIMER_CALIBRATION]);
        LF_StopCalibration(me);
        SCP_Transmit(&me->scpInstance, LF_CMD_CALIBRATE, NULL, 0);
        me->state = LF_IDLE;
        break;

    case LF_SIG_TIMER_TICK:
        LF_HandleTimerTick(me);

    default:
        break;
    }
}


/**
 * @brief Callback function for ADC data update.
 *
 * @param[in] data Pointer to the LineFollower instance.
 */
static void LF_DataUpdateCallback(void *data)
{
    LineFollower_T *const me = (LineFollower_T *const)data;

    LF_SendSignal(me, LF_SIG_ADC_DATA_UPDATED);
}

/**
 * @brief Sends debug data over the communication protocol.
 *
 * @param[in] packet Pointer to the SCP packet.
 * @param[in] context Pointer to the LineFollower instance.
 */
static void LF_SendDebugData(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const)context;

    memcpy(me->debugData.sensorsValues, me->sensorsInstance.adcBuffer, sizeof(me->sensorsInstance.adcBuffer));
    me->debugData.motorLeftVelocity = me->encoderLeft.velocity;
    me->debugData.motorRightVelocity = me->encoderRight.velocity;
    me->debugData.isSpeedReduced = LF_IsTimerOn(me->timers[LF_TIMER_SENSORS_STABILIZE]) ||
                                   LF_IsTimerOn(me->timers[LF_TIMER_REDUCED_SPEED]);

    SCP_Transmit(&me->scpInstance, LF_CMD_SEND_DEBUG_DATA, &me->debugData, sizeof(me->debugData));
}

/**
 * @brief Check and clamp motor speed.
 *
 * @param[in] speed Requested motor speed.
 * @return Clamped motor speed.
 */
static uint16_t LF_ClampMotorSpeed(float speed)
{
    if (speed < 0.0f)
    {
        return 0U;
    }
    else if (speed > LF_MAX_MOTOR_SPEED)
    {
        return LF_MAX_MOTOR_SPEED;
    }
    else
    {
        return (uint16_t)speed;
    }
}

/**
 * @brief Log the error.
 */
static void LF_LogError(const char *file, int line, LF_ErrorCode_T errorCode)
{
    __builtin_trap();
    HAL_NVIC_SystemReset();
}

/**
 * @brief Linefollower state machine main function.
 *
 * @param[in] me Pointer to the LineFollower instance.
 */
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
            /* Handle error state if necessary */
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

/**
 * @brief Sends a signal to the LineFollower state machine.
 *
 * @param[in] me Pointer to the LineFollower instance.
 * @param[in] sig Signal to send.
 */
void LF_SendSignal(LineFollower_T *const me, LF_Signal_T sig)
{
    if (!LF_SignalQueueEnqueue(&me->signals, sig))
    {
        /* TODO: Handle queue full */
    }
}

/**
 * @brief Timer callback function for debug mode.
 *
 * @param[in] context Pointer to the LineFollower instance.
 */
void LF_DebugModeTimerCallback(void *context)
{
    LineFollower_T *const me = (LineFollower_T *const)context;

    LF_SendSignal(me, LF_SIG_SEND_DEBUG_DATA);
}
