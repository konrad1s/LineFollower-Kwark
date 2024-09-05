#include "sensors.h"
#include "linefollower_config.h"
#include <stdbool.h>

void Sensors_Init(Sensors_Manager_T *manager,
                  const Sensors_Config_T *config,
                  uint16_t *adcBuffer,
                  Sensor_Instance_T *sensorInstances,
                  uint16_t sensorCount)
{
    if (manager == NULL || config == NULL || adcBuffer == NULL || sensorInstances == NULL)
    {
        return;
    }

    manager->adcBuffer = adcBuffer;
    manager->adcHandle = config->adcHandle;
    manager->sensors = sensorInstances;
    manager->sensorCount = sensorCount;

    for (uint16_t i = 0U; i < sensorCount; i++)
    {
        manager->sensors[i].isActive = false;
        manager->sensors[i].led = &config->ledConfig[i];
    }

    /* Start ADC in DMA mode */
    if (HAL_ADC_Start_DMA(manager->adcHandle, (uint32_t *)manager->adcBuffer, sensorCount) != HAL_OK)
    {
        /* TODO: Handle ADC initialization error */
    }
}

void Sensors_SetThreshold(Sensors_Manager_T *const manager, uint16_t threshold)
{
    manager->threshold = threshold;
}

void Sensors_UpdateState(Sensors_Manager_T *const manager)
{
    for (uint16_t i = 0U; i < manager->sensorCount; i++)
    {
        manager->sensors[i].isActive = (manager->adcBuffer[i] > manager->threshold);
    }
}

void Sensors_GetState(const Sensors_Manager_T *const manager, bool *state)
{
    for (uint16_t i = 0U; i < manager->sensorCount; i++)
    {
        state[i] = manager->sensors[i].isActive;
    }
}

void Sensors_UpdateLeds(const Sensors_Manager_T *const manager)
{
    for (uint16_t i = 0U; i < manager->sensorCount; i++)
    {
        Sensor_Instance_T *sensor = &manager->sensors[i];
        GPIO_PinState pinState = sensor->isActive ? GPIO_PIN_SET : GPIO_PIN_RESET;

        if (sensor->led != NULL)
        {
            HAL_GPIO_WritePin(sensor->led->port, sensor->led->pin, pinState);
        }
    }
}
