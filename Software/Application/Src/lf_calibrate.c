#include "nvm.h"
#include "linefollower_config.h"
#include "sensors.h"
#include "tb6612_motor.h"
#include "lf_calibrate.h"

#define CALIB_MIN_VALUE_IDX   0
#define CALIB_MAX_VALUE_IDX   1
#define DEFAULT_CALIBRATION_TIME 3000U
#define DEFAULT_MOTOR_SPEED   50U

typedef struct
{
    uint32_t startTime;
    uint16_t calibrationTime;
    uint16_t sensorValues[SENSORS_NUMBER][2];
    uint16_t motorSpeed;
} LF_CalibrationData_T;

static LF_CalibrationData_T LF_CalibrationData;

static void LF_ApplySensorThresholds(void)
{
    uint16_t thresholds[SENSORS_NUMBER];

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        uint16_t minValue = LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX];
        uint16_t maxValue = LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX];
        uint16_t range = maxValue - minValue;
        uint16_t threshold = minValue + (range / 2U);

        thresholds[i] = threshold;
    }

    Sensors_SetThresholds(thresholds);
}

static void LF_StopCalibration(LineFollower_T *const me)
{
    LF_CalibrationData.startTime = UINT32_MAX;

    TB6612Motor_Brake(&me->motorLeftConfig);
    TB6612Motor_Brake(&me->motorRightConfig);
    TB6612Motor_SetSpeed(&me->motorLeftConfig, 0U);
    TB6612Motor_SetSpeed(&me->motorRightConfig, 0U);
}

void LF_StartCalibration(LineFollower_T *const me)
{
    LF_CalibrationData.startTime = HAL_GetTick();
    LF_CalibrationData.calibrationTime = DEFAULT_CALIBRATION_TIME;
    LF_CalibrationData.motorSpeed = DEFAULT_MOTOR_SPEED;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX] = UINT16_MAX;
        LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX] = 0U;
    }

    TB6612Motor_ChangeDirection(&me->motorLeftConfig, MOTOR_FORWARD);
    TB6612Motor_ChangeDirection(&me->motorRightConfig, MOTOR_FORWARD);
    TB6612Motor_SetSpeed(&me->motorLeftConfig, LF_CalibrationData.motorSpeed);
    TB6612Motor_SetSpeed(&me->motorRightConfig, LF_CalibrationData.motorSpeed);
}

LF_CalibrationStatus_T LF_CalibrateSensors(LineFollower_T *const me)
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

    /* Check if calibration time has passed */
    uint32_t currentTime = HAL_GetTick();

    if ((currentTime - LF_CalibrationData.startTime) >= LF_CalibrationData.calibrationTime)
    {
        LF_ApplySensorThresholds();
        LF_StopCalibration(me);
        return LF_CALIBRATION_COMPLETE;
    }

    return LF_CALIBRATION_IN_PROGRESS;
}
