/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "sensors.h"
#include "cmsis_compiler.h"
#include <string.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define Sensors_EnterCritical() __disable_irq()
#define Sensors_ExitCritical()  __enable_irq()

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
static void Sensors_UpdateState(Sensors_Instance_T *const instance);

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/

/**
 * @brief Updates the state of the sensors based on ADC readings.
 *
 * @param[in,out] instance Pointer to the sensors instance.
 */
static void Sensors_UpdateState(Sensors_Instance_T *const instance)
{
    instance->anySensorDetectedLine = false;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        if (instance->adcBuffer[i] > instance->thresholds[i])
        {
            instance->anySensorDetectedLine = true;
            instance->sensors[i].isActive = true;
        }
        else
        {
            instance->sensors[i].isActive = false;
        }
    }
}

/**
 * @brief Initializes the sensor module.
 *
 * @param[in,out] instance         Pointer to the sensors instance.
 * @param[in]     callback         Callback function for data updates.
 * @param[in]     callbackContext  Context to pass to the callback function.
 *
 * @return 0 on success, -1 on failure.
 */
int Sensors_Init(Sensors_Instance_T *const instance,
                 Sensor_DataUpdatedCb_T callback,
                 void *callbackContext)
{
    if (instance == NULL || instance->config->adcHandle == NULL ||
        instance->config->ledConfig == NULL || instance->config->timer == NULL)
    {
        return -1;
    }

    instance->callback = callback;
    instance->callbackContext = callbackContext;
    instance->anySensorDetectedLine = false;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        instance->sensors[i].isActive = false;
        instance->thresholds[i] = 0xFFFFU;
    }

    /* Start ADC in DMA mode, triggered by timer */
    if (HAL_TIM_Base_Start(instance->config->timer) != HAL_OK)
    {
        return -1;
    }
    if (HAL_ADC_Start_DMA(instance->config->adcHandle, (uint32_t *)instance->adcBuffer, SENSORS_NUMBER) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief Sets the thresholds for sensor activation.
 *
 * @param[in,out] instance    Pointer to the sensors instance.
 * @param[in]     thresholds  Array of thresholds for each sensor.
 */
void Sensors_SetThresholds(Sensors_Instance_T *const instance, uint16_t *const thresholds)
{
    if (instance == NULL || thresholds == NULL)
    {
        return;
    }

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        instance->thresholds[i] = thresholds[i];
    }
}

/**
 * @brief Updates the state of the LEDs based on sensor activity.
 *
 * @param[in,out] instance Pointer to the sensors instance.
 */
void Sensors_UpdateLeds(Sensors_Instance_T *const instance)
{
    if (instance == NULL)
    {
        return;
    }

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        const Sensor_Led_T *const led = &instance->config->ledConfig[i];
        GPIO_PinState pinState = instance->sensors[i].isActive ? GPIO_PIN_SET : GPIO_PIN_RESET;

        if (led != NULL)
        {
            HAL_GPIO_WritePin(led->port, led->pin, pinState);
        }
    }
}

/**
 * @brief Calculates the error based on sensor readings.
 *
 * @param[in,out] instance    Pointer to the sensors instance.
 * @param[in]     nvmSensors  Pointer to the configuration from NVM.
 *
 * @return Calculated error value.
 */
float Sensors_CalculateError(Sensors_Instance_T *const instance, const NVM_Sensors_T *const nvmSensors)
{
    static float lastError = 0.0f;
    float currentError = 0.0f;
    int totalWeight = 0;
    int activeSensors = 0;

    if (instance == NULL || nvmSensors == NULL)
    {
        return lastError;
    }

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        instance->sensors[i].positionWeight = nvmSensors->weights[i];

        if (instance->sensors[i].isActive)
        {
            totalWeight += instance->sensors[i].positionWeight;
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
            currentError = nvmSensors->fallbackErrorNegative;
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

/**
 * @brief ADC conversion complete callback function.
 *
 * @param[in,out] instance Pointer to the sensors instance.
 * @param[in]     hadc     ADC handle.
 */
void Sensors_ADCConvCpltCallback(Sensors_Instance_T *const instance, ADC_HandleTypeDef *hadc)
{
    if (instance == NULL || instance->config->adcHandle != hadc)
    {
        return;
    }

    Sensors_UpdateState(instance);
    if (instance->callback != NULL)
    {
        instance->callback(instance->callbackContext);
    }
}
