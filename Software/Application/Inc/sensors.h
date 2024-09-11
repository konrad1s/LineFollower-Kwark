#ifndef __SENSORS__H__
#define __SENSORS__H__

#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "gpio.h"
#include "linefollower_config.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} Sensor_Led_T;

typedef struct
{
    bool isActive;
    int8_t positionWeight;
    Sensor_Led_T *led;
} Sensor_Instance_T;

void Sensors_Init(ADC_HandleTypeDef *const adcHandle,
                  Sensor_Led_T *const ledConfig,
                  Sensor_Instance_T *const sensorInstances,
                  Sensor_DataUpdatedCb_T callback);
void Sensors_SetThreshold(uint16_t threshold);
void Sensors_UpdateState(void);
void Sensors_GetState(bool *state);
void Sensors_UpdateLeds(void);
float Sensors_CalculateError(const NVM_Layout_T *nvm);


#endif /* __SENSORS__H__ */
