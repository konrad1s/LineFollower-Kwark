/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "nvm.h"
#include "linefollower_config.h"
#include "sensors.h"
#include "tb6612_motor.h"
#include "lf_calibrate.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define CALIB_MIN_VALUE_IDX       0
#define CALIB_MAX_VALUE_IDX       1
#define DEFAULT_CALIBRATION_TIME_MS 3000U
#define DEFAULT_MOTOR_SPEED       50U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef struct
{
    uint32_t elapsedTicks;
    uint16_t sensorValues[SENSORS_NUMBER][2];
    uint16_t motorSpeed;
} LF_CalibrationData_T;

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static int LF_ApplySensorThresholds(LineFollower_T *const me);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/
static LF_CalibrationData_T LF_CalibrationData;

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
static int LF_ApplySensorThresholds(LineFollower_T *const me)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        uint16_t minValue = LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX];
        uint16_t maxValue = LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX];
        uint16_t range = maxValue - minValue;
        uint16_t threshold = minValue + (range / 2U);

        me->nvmBlock->sensors.thresholds[i] = threshold;
    }

    Sensors_SetThresholds(me->nvmBlock->sensors.thresholds);
    return NVM_Write(&me->nvmInstance);
}

static void LF_StopCalibration(LineFollower_T *const me)
{
    LF_CalibrationData.elapsedTicks = 0U;

    TB6612Motor_Brake(me->motorLeftConfig);
    TB6612Motor_Brake(me->motorRightConfig);
    TB6612Motor_SetSpeed(me->motorLeftConfig, 0U);
    TB6612Motor_SetSpeed(me->motorRightConfig, 0U);
}

void LF_StartCalibration(LineFollower_T *const me)
{
    LF_CalibrationData.elapsedTicks = 0U;
    LF_CalibrationData.motorSpeed = DEFAULT_MOTOR_SPEED;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX] = UINT16_MAX;
        LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX] = 0U;
    }

    TB6612Motor_ChangeDirection(me->motorLeftConfig, MOTOR_FORWARD);
    TB6612Motor_ChangeDirection(me->motorRightConfig, MOTOR_FORWARD);
    TB6612Motor_SetSpeed(me->motorLeftConfig, LF_CalibrationData.motorSpeed);
    TB6612Motor_SetSpeed(me->motorRightConfig, LF_CalibrationData.motorSpeed);
}

void LF_UpdateCalibrationData(LineFollower_T *const me)
{
    uint16_t sensorRawData[SENSORS_NUMBER] = {0U};

    Sensors_GetRawData(sensorRawData);

    /* Update min and max values, TODO: consider to apply moving average filter */
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        if (sensorRawData[i] < LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX])
        {
            LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX] = sensorRawData[i];
        }

        if (sensorRawData[i] > LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX])
        {
            LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX] = sensorRawData[i];
        }
    }
}

LF_CalibrationStatus_T LF_UpdateCalibrationTimer(LineFollower_T *const me)
{
    LF_CalibrationData.elapsedTicks++;

    if (LF_CalibrationData.elapsedTicks >= DEFAULT_CALIBRATION_TIME_MS)
    {
        LF_StopCalibration(me);

        if (0 == LF_ApplySensorThresholds(me))
        {
            return LF_CALIBRATION_COMPLETE;
        }
        else
        {
            return LF_CALIBRATION_ERROR;
        }
    }

    return LF_CALIBRATION_IN_PROGRESS;
}
