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
    .pidStgEncoderLeft = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 1000.0f,
        .output_min = -1000.0f,
    },
    .pidStgEncoderRight = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 1000.0f,
        .output_min = -1000.0f,
    },
    .sensors = {
        .weights = {-7, -5, -4, -2, -1, 0, 0, 1, 2, 4, 5, 7},
        .thresholds = {3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U, 3200U},
        .errorThreshold = 1.0f,
        .fallbackErrorPositive = 15.0f,
        .fallbackErrorNegative = -15.0f
    },
    .targetSpeed = 1.0f
};

/* --------------------------------- SENSORS CONFIG --------------------------------- */

const Sensor_Led_T sensorLeds[SENSORS_NUMBER] = {
    {LED1_GPIO_Port, LED1_Pin},
    {LED2_GPIO_Port, LED2_Pin},
    {LED3_GPIO_Port, LED3_Pin},
    {LED4_GPIO_Port, LED4_Pin},
    {LED5_GPIO_Port, LED5_Pin},
    {LED6_GPIO_Port, LED6_Pin},
    {LED7_GPIO_Port, LED7_Pin},
    {LED8_GPIO_Port, LED8_Pin},
    {LED9_GPIO_Port, LED9_Pin},
    {LED10_GPIO_Port, LED10_Pin},
    {LED11_GPIO_Port, LED11_Pin},
    {LED12_GPIO_Port, LED12_Pin},
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

/* -------------------------------- ENCODERS CONFIG -------------------------------- */

const Encoder_Settings_T encoderSettings = {
    .gearRatio = 9.96f,
    .wheelDiameter = 0.0245f
};
