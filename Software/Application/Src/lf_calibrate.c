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
#define DEFAULT_MOTOR_SPEED       150U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef struct
{
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

    Sensors_SetThresholds(&me->sensorsInstance, me->nvmBlock->sensors.thresholds);
    return NVM_Write(&me->nvmInstance);
}

void LF_StopCalibration(LineFollower_T *const me)
{
    TB6612Motor_Brake(me->motorLeft);
    TB6612Motor_Brake(me->motorRight);
    TB6612Motor_SetSpeed(me->motorLeft, 0U);
    TB6612Motor_SetSpeed(me->motorRight, 0U);

    (void)LF_ApplySensorThresholds(me);
}

void LF_StartCalibration(LineFollower_T *const me)
{
    LF_CalibrationData.motorSpeed = DEFAULT_MOTOR_SPEED;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX] = UINT16_MAX;
        LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX] = 0U;
    }

    TB6612Motor_ChangeDirection(me->motorLeft, MOTOR_FORWARD);
    TB6612Motor_ChangeDirection(me->motorRight, MOTOR_BACKWARD);
    TB6612Motor_SetSpeed(me->motorLeft, LF_CalibrationData.motorSpeed);
    TB6612Motor_SetSpeed(me->motorRight, LF_CalibrationData.motorSpeed);
}

void LF_UpdateCalibrationData(LineFollower_T *const me)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        const uint16_t sensorVal = me->sensorsInstance.adcBuffer[i];

        if (sensorVal < LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX])
        {
            LF_CalibrationData.sensorValues[i][CALIB_MIN_VALUE_IDX] = sensorVal;
        }

        if (sensorVal > LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX])
        {
            LF_CalibrationData.sensorValues[i][CALIB_MAX_VALUE_IDX] = sensorVal;
        }
    }
}
