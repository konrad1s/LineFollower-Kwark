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

float Sensors_CalculateError(const Sensors_Manager_T *manager, const NVM_Layout_T *nvm)
{
    if (manager == NULL || manager->sensors == NULL)
    {
        return 0.0f;
    }

    static float lastError = 0.0f;
    float currentError = 0.0f;
    int totalWeight = 0;
    int activeSensors = 0;

    for (uint16_t i = 0U; i < manager->sensorCount; i++)
    {
        if (manager->sensors[i].isActive)
        {
            totalWeight += manager->sensors[i].positionWeight;
            activeSensors++;
        }
    }

    if (activeSensors == 0)
    {
        /* If no sensors are active, return the fallback error based on the last known error */
        if (lastError > nvm->errorThreshold)
        {
            currentError = nvm->fallbackErrorPositive;
        }
        else if (lastError < nvm->errorThreshold)
        {
            currentError = nvm->fallbackErrorPositive;
        }
        else
        {
            return lastError;
        }
    }
    else
    {
        /* Calculate the current error as a weighted average of active sensors */
        currentError = (float)totalWeight / (float)activeSensors;
    }

    lastError = currentError;

    return currentError;
}
