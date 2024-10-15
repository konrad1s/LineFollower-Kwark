#ifndef __LINEFOLLOWER_CONFIG_H__
#define __LINEFOLLOWER_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>
#include "pid.h"
#include "scp.h"
#include "tb6612_motor.h"

#define NVM_SECTOR_USED  FLASH_SECTOR_2

#define SCP_BUFFER_SIZE  256U

#define SENSORS_NUMBER   (12U)

typedef struct
{
    int8_t weights[SENSORS_NUMBER];
    uint16_t thresholds[SENSORS_NUMBER];
    float errorThreshold;
    float fallbackErrorPositive;
    float fallbackErrorNegative;
} NVM_Sensors_T;

typedef struct
{
    PID_Settings_T pidStgSensor;
    PID_Settings_T pidStgEncoderLeft;
    PID_Settings_T pidStgEncoderRight;
    NVM_Sensors_T sensors;
    float targetSpeed;
} NVM_Layout_T;

extern const NVM_Layout_T NvmDefaultData;
extern const TB6612MotorDriver_T LeftMotor;
extern const TB6612MotorDriver_T RightMotor;

#endif /* __LINEFOLLOWER_CONFIG_H__ */
