#ifndef __ENCODER_H__
#define __ENCODER_H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "stm32f7xx_hal.h"
#include <stdint.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef struct
{
    float gearRatio;
    float wheelDiameter;
    uint32_t pulsesPerRevolution;
} Encoder_Settings_T;

typedef struct
{
    const Encoder_Settings_T *const settings;

    TIM_HandleTypeDef *htim;
    int32_t count;
    int32_t countPrev;
    int32_t deltaCount;
    uint32_t timerMax;
    float velocity; /* m/s */
} Encoder_Instance_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
int Encoder_Init(Encoder_Instance_T *const encoder);
int32_t Encoder_GetCount(Encoder_Instance_T *const encoder);
void Encoder_ResetCount(Encoder_Instance_T *const encoder);
void Encoder_Update(Encoder_Instance_T *const encoder, float dt);

#endif /* __ENCODER_H__ */
