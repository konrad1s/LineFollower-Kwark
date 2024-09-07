#ifndef __SENSORS__H__
#define __SENSORS__H__

#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "gpio.h"

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
typedef struct
{
    ADC_HandleTypeDef *adcHandle;
    Sensor_Led_T *ledConfig;
} Sensors_Config_T;

typedef struct
{
    uint16_t *adcBuffer;         // Pointer to ADC data buffer
    ADC_HandleTypeDef *adcHandle;// Pointer to the ADC handler
    Sensor_Instance_T *sensors;  // Pointer to sensor instances
    uint16_t sensorCount;        // Number of sensors
    uint16_t threshold;          // Threshold value for sensor activation
} Sensors_Manager_T;

void Sensors_Init(Sensors_Manager_T *manager, const Sensors_Config_T *config, uint16_t *adcBuffer,
                  Sensor_Instance_T *sensorInstances, uint16_t sensorCount);
void Sensors_SetThreshold(Sensors_Manager_T *const sensorsManager, uint16_t threshold);
void Sensors_UpdateState(Sensors_Manager_T *const sensorsManager);
void Sensors_GetState(const Sensors_Manager_T *const sensorsManager, bool *state);
void Sensors_UpdateLeds(const Sensors_Manager_T *const sensorsManager);


#endif /* __SENSORS__H__ */
