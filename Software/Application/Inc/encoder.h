#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef struct
{
    float gearRatio;
    float wheelDiameter;
} Encoder_Settings_T;

typedef struct
{
    Encoder_Settings_T *settings;

    TIM_HandleTypeDef *htim;
    int32_t count;
    int32_t countPrev;
    int32_t deltaCount;
    uint32_t timerMax;
    float velocity;
} Encoder_Instance_T;

int Encoder_Init(Encoder_Instance_T *const encoder);
int32_t Encoder_GetCount(Encoder_Instance_T *const encoder);
void Encoder_ResetCount(Encoder_Instance_T *const encoder);
void Encoder_Update(Encoder_Instance_T *const encoder, float dt);

#endif /* __ENCODER_H__ */
