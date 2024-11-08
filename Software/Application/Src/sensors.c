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
static bool Sensors_UpdateSlidingWindow(Sensors_Instance_T *const instance, uint16_t sensorIndex,
                                        uint8_t *activeCount, uint8_t *windowStart,
                                        uint8_t maxStartIndex, uint8_t sideStartIndex);
static void Sensors_UpdateState(Sensors_Instance_T *const instance);

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/

/**
 * @brief Updates the sliding window for a side during each iteration.
 *
 * @param[in]     instance          Pointer to the sensors instance.
 * @param[in]     sensorIndex       Actual sensor index in the sensors array.
 * @param[in,out] activeCount       Pointer to the active count for the side.
 * @param[in,out] windowStart       Pointer to the window start index for the side.
 * @param[in]     maxStartIndex     Maximum start index for the sliding window.
 * @param[in]     sideStartIndex    Starting index of the side (0 for left, sensorsPerSide for right).
 * @return        true if a right angle is detected; false otherwise.
 */
static bool Sensors_UpdateSlidingWindow(Sensors_Instance_T *const instance, uint16_t sensorIndex,
                                        uint8_t *activeCount, uint8_t *windowStart,
                                        uint8_t maxStartIndex, uint8_t sideStartIndex)
{
    bool rightAngleDetected = false;
    uint8_t windowSize = instance->config->rightAngleWindow;
    uint8_t relativeIndex = sensorIndex - sideStartIndex;

    if (relativeIndex < windowSize)
    {
        /* Initialize activeCount for the first window */
        *activeCount += instance->sensors[sensorIndex].isActive ? 1 : 0;
        if ((relativeIndex == windowSize - 1) && (*activeCount == windowSize))
        {
            rightAngleDetected = true;
        }
    }
    else if (*windowStart <= maxStartIndex)
    {
        /* Slide the window */
        uint8_t outgoingSensorIndex = sideStartIndex + *windowStart;
        *activeCount -= instance->sensors[outgoingSensorIndex].isActive ? 1 : 0;
        *activeCount += instance->sensors[sensorIndex].isActive ? 1 : 0;
        (*windowStart)++;

        if (*activeCount == windowSize)
        {
            rightAngleDetected = true;
        }
    }

    return rightAngleDetected;
}

/**
 * @brief Checks if only 2 middle sensors are active.
 *
 * @param[in] instance     Pointer to the sensors instance.
 * @param[in] activeSensors Total number of active sensors.
 * @return    true if a straight line is detected; false otherwise.
 */
static bool Sensors_CheckStraightLine(Sensors_Instance_T *const instance, uint8_t activeSensors)
{
    bool straightLine = false;
    uint16_t midIndex1 = (SENSORS_NUMBER / 2) - 1;
    uint16_t midIndex2 = SENSORS_NUMBER / 2;

    if ((activeSensors == 2) &&
        ((instance->sensors[midIndex1].isActive) && (instance->sensors[midIndex2].isActive)))
    {
        straightLine = true;
    }

    return straightLine;
}

/**
 * @brief Determines if the stabilization condition is met
 *
 * @param[in] allOutsideMiddleInactive  True if all sensors outside the middle are inactive.
 * @param[in] middleActiveCount         Count of active sensors within the middle region.
 * @return    true if stabilization is met; false otherwise.
 */
static bool Sensors_CheckStabilization(bool allOutsideMiddleInactive, uint8_t middleActiveCount)
{
    return allOutsideMiddleInactive && (middleActiveCount >= 1);
}

/**
 * @brief Updates the state of the sensors based on ADC readings.
 *
 * @param[in,out] instance Pointer to the sensors instance.
 */
static void Sensors_UpdateState(Sensors_Instance_T *const instance)
{
    const uint8_t rightAngleWindowSize = instance->config->rightAngleWindow;
    const uint8_t stabilizeWindowSize = instance->config->stabilizeWindow;
    const uint16_t stabilizeStartIdx = (SENSORS_NUMBER - stabilizeWindowSize) / 2;
    const uint16_t stabilizeEnddx = stabilizeStartIdx + stabilizeWindowSize - 1;

    uint8_t sensorsPerSide = SENSORS_NUMBER / 2;
    uint8_t leftActiveCount = 0, rightActiveCount = 0;
    uint8_t leftWindowStart = 0, rightWindowStart = 0;
    uint8_t maxStartIndex = sensorsPerSide - rightAngleWindowSize;

    bool leftHasRightAngle = false, rightHasRightAngle = false;
    uint8_t activeSensorCount = 0;
    uint8_t middleActiveCount = 0;
    bool allOutsideMiddleInactive = true;

    instance->anySensorDetectedLine = false;

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        bool isActive = instance->adcBuffer[i] > instance->thresholds[i];
        instance->sensors[i].isActive = isActive;
        if (isActive)
        {
            instance->anySensorDetectedLine = true;
            activeSensorCount++;

            if (i >= stabilizeStartIdx && i <= stabilizeEnddx)
            {
                middleActiveCount++;
            }
            else
            {
                allOutsideMiddleInactive = false;
            }
        }

        /* Right angle detection */
        if ((!leftHasRightAngle) && (!rightHasRightAngle))
        {
            if (i < sensorsPerSide)
            {
                leftHasRightAngle = Sensors_UpdateSlidingWindow(instance, i, &leftActiveCount,
                                                                &leftWindowStart, maxStartIndex, 0);
            }
            else
            {
                rightHasRightAngle = Sensors_UpdateSlidingWindow(instance, i, &rightActiveCount,
                                                                 &rightWindowStart, maxStartIndex, sensorsPerSide);
            }
        }
    }

    instance->rightAngleDetected = (leftHasRightAngle || rightHasRightAngle);
    instance->straightLineDetected = Sensors_CheckStraightLine(instance, activeSensorCount);
    instance->stabilizeDetected = Sensors_CheckStabilization(allOutsideMiddleInactive, middleActiveCount);
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
