#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "usart.h"
#include "tim.h"
#include "sensors.h"
#include "stm32f7xx_hal.h"

/* TODO: Dummy values, replace it after tests */

/* ----------------------------------- NVM CONFIG ----------------------------------- */

const NVM_Layout_T NvmDefaultData = {
    .pidStgSensor = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 100.0f,
        .output_min = -100.0f,
    },
    .pidStgMotorLeft = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 100.0f,
        .output_min = -100.0f,
    },
    .pidStgMotorRight = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 100.0f,
        .output_min = -100.0f,
    },
    .sensors = {
        .weights = { -7, -5, -4, -2, -1, 0, 0, 1, 2, 4, 5, 7},
        .thresholds = { 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U},
        .errorThreshold = 1.0f,
        .fallbackErrorPositive = 15.0f,
        .fallbackErrorNegative = -15.0f
    }

};

/* --------------------------------- SENSORS CONFIG --------------------------------- */

const Sensor_Led_T sensorLeds[SENSORS_NUMBER] = {
    {GPIOC, LED12_Pin},
    {GPIOC, LED11_Pin},
    {GPIOC, LED10_Pin},
    {GPIOC, LED9_Pin},
    {GPIOC, LED8_Pin},
    {GPIOC, LED7_Pin},
    {GPIOB, LED6_Pin},
    {GPIOB, LED5_Pin},
    {GPIOB, LED4_Pin},
    {GPIOB, LED3_Pin},
    {GPIOB, LED2_Pin},
    {GPIOB, LED1_Pin},
};

/* --------------------------------- MOTORS CONFIG --------------------------------- */

const TB6612MotorDriver_T LeftMotor = {
    .in1 = {GPIOA, MOTOR1_AIN_Pin},
    .in2 = {GPIOA, MOTOR1_BIN_Pin},
    .pwmTimer = &htim1,
    .pwmChannel = TIM_CHANNEL_4
};

const TB6612MotorDriver_T RightMotor = {
    .in1 = {GPIOB, MOTOR2_AIN_Pin},
    .in2 = {GPIOD, MOTOR2_BIN_Pin},
    .pwmTimer = &htim3,
    .pwmChannel = TIM_CHANNEL_1
};
