#ifndef __LINEFOLLOWER_CONFIG_H__
#define __LINEFOLLOWER_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>
#include "pid.h"
#include "scp.h"
#include "tb6612_motor.h"

#define NVM_SECTOR_USED  FLASH_SECTOR_7

#define SCP_BUFFER_SIZE  128U

#define SENSORS_NUMBER   (12U)

typedef struct
{
    PID_Settings_T pidStgSensor;
    PID_Settings_T pidStgMotorLeft;
    PID_Settings_T pidStgMotorRight;

    int8_t sensorWeights[SENSORS_NUMBER];
    float errorThreshold;
    float fallbackErrorPositive;
    float fallbackErrorNegative;
} NVM_Layout_T;

extern const NVM_Layout_T NvmDefaultData;
extern const SCP_Config_T ScpConfig;
extern const TB6612MotorDriver_T LeftMotor;
extern const TB6612MotorDriver_T RightMotor;
extern void Sensors_Config_Init(ADC_HandleTypeDef *adcHandle, const int8_t *weights);

#endif /* __LINEFOLLOWER_CONFIG_H__ */
