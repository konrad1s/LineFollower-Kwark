#ifndef __SENSORS_H__
#define __SENSORS_H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "gpio.h"
#include "linefollower_config.h"

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} Sensor_Led_T;

typedef struct
{
    bool isActive;
    int8_t positionWeight;
} Sensor_Instance_T;

typedef void (*Sensor_DataUpdatedCb_T)(void *context);

typedef struct
{
    ADC_HandleTypeDef *adcHandle;
    const Sensor_Led_T *ledConfig;
    TIM_HandleTypeDef *timer;
    uint8_t rightAngleWindow;
} Sensors_Config_T;

typedef struct
{
    uint16_t adcBuffer[SENSORS_NUMBER];
    const Sensors_Config_T *const config;
    Sensor_Instance_T sensors[SENSORS_NUMBER];
    uint16_t thresholds[SENSORS_NUMBER];
    bool anySensorDetectedLine;
    bool rightAngleDetected;
    Sensor_DataUpdatedCb_T callback;
    void *callbackContext;
} Sensors_Instance_T;

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/

int Sensors_Init(Sensors_Instance_T *const instance,
                 Sensor_DataUpdatedCb_T callback,
                 void *callbackContext);
void Sensors_SetThresholds(Sensors_Instance_T *const instance, uint16_t *const thresholds);
void Sensors_GetRawData(Sensors_Instance_T *const instance, uint16_t *data);
void Sensors_UpdateLeds(Sensors_Instance_T *const instance);
float Sensors_CalculateError(Sensors_Instance_T *const instance, const NVM_Sensors_T *const nvmSensors);
void Sensors_ADCConvCpltCallback(Sensors_Instance_T *const instance, ADC_HandleTypeDef *hadc);

#endif /* __SENSORS_H__ */
