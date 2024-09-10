#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "usart.h"
#include "tim.h"

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
    .sensorWeights = {
        -7, -5, -4, -2, -1, 0, 0, 1, 2, 4, 5, 7
    },
    .errorThreshold = 1.0f,
    .fallbackErrorPositive = 15.0f,
    .fallbackErrorNegative = -15.0f
};

/* ----------------------------------- SCP CONFIG ----------------------------------- */

static uint8_t scpBuffer[SCP_BUFFER_SIZE];

const SCP_Config_T ScpConfig = {
        .buffer = scpBuffer,
        .size = SCP_BUFFER_SIZE,
        .huart = &huart4,
        .commands = lineFollowerCommands,
        .numCommands = sizeof(lineFollowerCommands) / sizeof(lineFollowerCommands[0]),
        .errorHandler = NULL,
    };

/* --------------------------------- SENSORS CONFIG --------------------------------- */

static Sensor_Instance_T sensorInstances[SENSORS_NUMBER];
static uint16_t sensorAdcBuffer[SENSORS_NUMBER];
static Sensor_Led_T sensorLeds[SENSORS_NUMBER] = {
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

void Sensors_Config_Init(Sensors_Manager_T *manager, ADC_HandleTypeDef *adcHandle, const int8_t *weights)
{
    Sensors_Config_T config = {
        .adcHandle = adcHandle,
        .ledConfig = sensorLeds
    };

    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        sensorInstances[i].positionWeight = weights[i];
        sensorInstances[i].isActive = false;
    }

    Sensors_Init(manager, &config, sensorAdcBuffer, sensorInstances, SENSORS_NUMBER);
}

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
