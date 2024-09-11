#include "sensors.h"
#include <stdbool.h>
#include "tim.h"

typedef struct
{
    uint16_t adcBuffer[SENSORS_NUMBER];
    ADC_HandleTypeDef *adcHandle;
    Sensor_Instance_T *sensors;
    uint16_t threshold;
    Sensor_DataUpdatedCb_T callback;
} Sensors_Manager_T;

static Sensors_Manager_T SensorsManager;

void Sensors_Init(ADC_HandleTypeDef *const adcHandle,
                  Sensor_Led_T *const ledConfig,
                  Sensor_Instance_T *const sensorInstances,
                  Sensor_DataUpdatedCb_T callback)
{
    if (adcHandle == NULL || sensorInstances == NULL || ledConfig == NULL)
    {
        return; 
    }

    SensorsManager.adcHandle = adcHandle;
    SensorsManager.sensors = sensorInstances;
    SensorsManager.threshold = 0U;
    SensorsManager.callback = callback;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        SensorsManager.sensors[i].isActive = false;
        SensorsManager.sensors[i].led = &ledConfig[i];
    }

    /* Start ADC in DMA mode */
    HAL_TIM_Base_Start(&htim2);
    if (HAL_ADC_Start_DMA(SensorsManager.adcHandle, (uint32_t *)SensorsManager.adcBuffer, SENSORS_NUMBER) != HAL_OK)
    {
       
    }
}

void Sensors_SetThreshold(uint16_t threshold)
{
    SensorsManager.threshold = threshold;
}

void Sensors_UpdateState(void)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        SensorsManager.sensors[i].isActive = (SensorsManager.adcBuffer[i] > SensorsManager.threshold);
    }
}

void Sensors_GetState(bool *state)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        state[i] = SensorsManager.sensors[i].isActive;
    }
}

void Sensors_UpdateLeds(void)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        Sensor_Instance_T *sensor = &SensorsManager.sensors[i];
        GPIO_PinState pinState = sensor->isActive ? GPIO_PIN_SET : GPIO_PIN_RESET;

        if (sensor->led != NULL)
        {
            HAL_GPIO_WritePin(sensor->led->port, sensor->led->pin, pinState);
        }
    }
}

float Sensors_CalculateError(const NVM_Layout_T *nvm)
{
    static float lastError = 0.0f;
    float currentError = 0.0f;
    int totalWeight = 0;
    int activeSensors = 0;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        if (SensorsManager.sensors[i].isActive)
        {
            totalWeight += SensorsManager.sensors[i].positionWeight;
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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (SensorsManager.adcHandle == hadc)
    {
        Sensors_UpdateState();
        SensorsManager.callback();
    }
}
