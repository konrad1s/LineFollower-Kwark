#ifndef __TB6612FNG_MOTOR_H__
#define __TB6612FNG_MOTOR_H__

#include "stm32f7xx_hal.h"

typedef enum
{
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_STOP,
    MOTOR_BRAKE
} TB6612MotorDirection_T;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} TB6612MotorPin_T;

typedef struct
{
    TB6612MotorPin_T in1;
    TB6612MotorPin_T in2;
    TIM_HandleTypeDef *pwmTimer;
    uint32_t pwmChannel;
} TB6612MotorDriver_T;

int TB6612Motor_Init(const TB6612MotorDriver_T *const driver);
void TB6612Motor_ChangeDirection(const TB6612MotorDriver_T *const driver, TB6612MotorDirection_T direction);
void TB6612Motor_SetSpeed(const TB6612MotorDriver_T *const driver, uint16_t speed);
void TB6612Motor_Stop(const TB6612MotorDriver_T *const driver);
void TB6612Motor_Brake(const TB6612MotorDriver_T *const driver);

#endif /* __TB6612FNG_MOTOR_H__ */
