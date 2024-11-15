/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "usart.h"
#include "tim.h"
#include "sensors.h"
#include "stm32f7xx_hal.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/

const NVM_Layout_T NvmDefaultData = {
    .pidStgSensor = {
        .kp = 0.1f,
        .ki = 0.0f,
        .kd = 0.0f,
        .integral_max = 1.0f,
        .integral_min = -1.0f,
        .output_max = 1.5f,
        .output_min = -1.5f,
    },
    .pidStgEncoderLeft = {
        .kp = 984.0f,
        .ki = 21.34f,
        .kd = 1200.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 1000.0f,
        .output_min = 0.0f,
    },
    .pidStgEncoderRight = {
        .kp = 984.0f,
        .ki = 21.34f,
        .kd = 1200.0f,
        .integral_max = 100.0f,
        .integral_min = -100.0f,
        .output_max = 1000.0f,
        .output_min = 0.0f,
    },
    .sensors = {
        .weights = {-8, -6, -4, -2, -1, 0, 0, 1, 2, 4, 6, 8},
        .thresholds = {1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U},
        .errorThreshold = 1.0f,
        .fallbackErrorPositive = 10.0f,
        .fallbackErrorNegative = -10.0f
    },
    .targetSpeed = 1.3f,
    .timerTimeout = {
        [LF_TIMER_NO_LINE_DETECTED]= 1000U,
        [LF_TIMER_REDUCED_SPEED]= 300U,
        [LF_TIMER_SENSORS_STABILIZE]= 500U,
        [LF_TIMER_CALIBRATION] = 3000u
    }
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

const Sensors_Config_T sensorsConfig = {
    .adcHandle = &hadc1,
    .ledConfig = sensorLeds,
    .timer = &htim2,
    .rightAngleWindow = 4,
    .stabilizeWindow = 4
};

/* --------------------------------- MOTORS CONFIG --------------------------------- */
const TB6612MotorDriver_T LeftMotor = 
{
    .in1 = {GPIOA, MOTOR1_AIN_Pin},
    .in2 = {GPIOA, MOTOR1_BIN_Pin},
    .pwmTimer = &htim1,
    .pwmChannel = TIM_CHANNEL_4
};

const TB6612MotorDriver_T RightMotor =
{
    .in1 = {GPIOB, MOTOR2_AIN_Pin},
    .in2 = {GPIOD, MOTOR2_BIN_Pin},
    .pwmTimer = &htim3,
    .pwmChannel = TIM_CHANNEL_1
};

/* -------------------------------- ENCODERS CONFIG -------------------------------- */
const Encoder_Settings_T encoderSettings = {
    .gearRatio = 9.96f,
    .wheelDiameter = 0.0245f,
    .pulsesPerRevolution = 512
};


/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
