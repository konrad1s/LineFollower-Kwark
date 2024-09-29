#include <stdbool.h>
#include "sensors.h"
#include "tim.h"

#define Sensors_EnterCritical() __disable_irq()
#define Sensors_ExitCritical() __enable_irq()

typedef struct
{
    uint16_t adcBuffer[SENSORS_NUMBER];
    ADC_HandleTypeDef *adcHandle;
    Sensor_Instance_T *sensors;
    uint16_t thresholds[SENSORS_NUMBER];
    Sensor_DataUpdatedCb_T callback;
    void *callbackContext;
} Sensors_Manager_T;

static Sensors_Manager_T SensorsManager;

static void Sensors_UpdateState(void)
{
    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        SensorsManager.sensors[i].isActive = (SensorsManager.adcBuffer[i] > SensorsManager.thresholds[i]);
    }
}

int Sensors_Init(ADC_HandleTypeDef *const adcHandle,
                 const Sensor_Led_T *const ledConfig,
                 Sensor_Instance_T *const sensorInstances,
                 Sensor_DataUpdatedCb_T callback,
                 void *callbackContext)
{
    if (adcHandle == NULL || sensorInstances == NULL || ledConfig == NULL)
    {
        return -1;
    }

    SensorsManager.adcHandle = adcHandle;
    SensorsManager.sensors = sensorInstances;
    SensorsManager.callback = callback;
    SensorsManager.callbackContext = callbackContext;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        SensorsManager.sensors[i].isActive = false;
        SensorsManager.sensors[i].led = &ledConfig[i];
        SensorsManager.thresholds[i] = 0xFFFFU;
    }

    /* Start ADC in DMA mode, trigger by timer */
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        return -1;
    }
    if (HAL_ADC_Start_DMA(SensorsManager.adcHandle, (uint32_t *)SensorsManager.adcBuffer, SENSORS_NUMBER) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

void Sensors_SetThresholds(uint16_t *const thresholds)
{
    if (thresholds == NULL)
    {
        return;
    }

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        SensorsManager.thresholds[i] = thresholds[i];
    }
}

void Sensors_GetRawData(uint16_t *data)
{
    if (data == NULL)
    {
        return;
    }

    Sensors_EnterCritical();

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        data[i] = SensorsManager.adcBuffer[i];
    }

    Sensors_ExitCritical();
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

float Sensors_CalculateError(const NVM_Sensors_T *const nvmSensors)
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
        if (lastError > nvmSensors->errorThreshold)
        {
            currentError = nvmSensors->fallbackErrorPositive;
        }
        else if (lastError < nvmSensors->errorThreshold)
        {
            currentError = nvmSensors->fallbackErrorPositive;
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
        SensorsManager.callback(SensorsManager.callbackContext);
    }
}
