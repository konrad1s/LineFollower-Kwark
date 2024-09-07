#ifndef __LINEFOLLOWER_CONFIG_H__
#define __LINEFOLLOWER_CONFIG_H__

#include "pid.h"
#include "scp.h"
#include "sensors.h"

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
extern void Sensors_Config_Init(Sensors_Manager_T *manager, ADC_HandleTypeDef *adcHandle, const int8_t *weights);

#endif /* __LINEFOLLOWER_CONFIG_H__ */
