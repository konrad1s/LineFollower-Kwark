#include "encoder.h"
#include <math.h>

#define PI 3.14159265358979323846f

/**
 * @brief Initializes the encoder interface
 * @param encoder Pointer to the Encoder_Instance_T structure
 * @return 0 on success, -1 on failure
 */
int Encoder_Init(Encoder_Instance_T *const encoder)
{
    if (encoder == NULL || encoder->htim == NULL)
    {
        return -1;
    }

    if (HAL_TIM_Encoder_Start(encoder->htim, TIM_CHANNEL_ALL) != HAL_OK)
    {
        return -1;
    }

    encoder->timerMax = __HAL_TIM_GET_AUTORELOAD(encoder->htim);

    encoder->count = 0;
    encoder->countPrev = 0;
    encoder->deltaCount = 0;
    encoder->settings->velocity = 0.0f;

    return 0;
}

/**
 * @brief Reads the encoder count
 * @param encoder Pointer to the Encoder_Instance_T structure
 * @return The current count value
 */
int32_t Encoder_GetCount(Encoder_Instance_T *const encoder)
{
    if (encoder == NULL)
    {
        return 0;
    }

    int32_t rawCount = (int32_t)__HAL_TIM_GET_COUNTER(encoder->htim);

    return rawCount;
}

/**
 * @brief Resets the encoder count to zero
 * @param encoder Pointer to the Encoder_Instance_T structure
 */
void Encoder_ResetCount(Encoder_Instance_T *const encoder)
{
    if (encoder == NULL)
    {
        return;
    }

    __HAL_TIM_SET_COUNTER(encoder->htim, 0);
    encoder->count = 0;
    encoder->countPrev = 0;
    encoder->deltaCount = 0;
}

/**
 * @brief Updates the encoder data, should be called periodically
 * @param encoder Pointer to the Encoder_Instance_T structure
 * @param dt Time interval since last update in seconds
 */
void Encoder_Update(Encoder_Instance_T *const encoder, float dt)
{
    if (encoder == NULL || dt <= 0.0f)
    {
        return;
    }

    int32_t rawCount = (int32_t)__HAL_TIM_GET_COUNTER(encoder->htim);
    int32_t delta;

    /* Calculate delta count considering possible overflows */
    delta = rawCount - encoder->countPrev;

    /* Handle timer overflow (both directions) */
    if (delta > (int32_t)(encoder->timerMax / 2))
    {
        /* Counter overflowed in negative direction */
        delta -= (int32_t)(encoder->timerMax + 1);
    }
    else if (delta < -(int32_t)(encoder->timerMax / 2))
    {
        /* Counter overflowed in positive direction */
        delta += (int32_t)(encoder->timerMax + 1);
    }

    encoder->deltaCount = delta;
    encoder->count += delta;
    encoder->countPrev = rawCount;

    /* Calculate wheel rotations considering gearbox ratio */
    float motorRotations = (float)(encoder->deltaCount) / encoder->htim->Init.Period;
    float wheelRotations = motorRotations / encoder->settings->gearRatio;

    /* Calculate linear distance traveled */
    float distance = wheelRotations * PI * encoder->settings->wheelDiameter;

    /* Calculate settings->velocity (distance per time interval) */
    encoder->settings->velocity = distance / dt;
}
