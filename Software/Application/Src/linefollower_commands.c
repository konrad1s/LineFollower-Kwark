#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "sensors.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern NVM_Layout_T NVM_Block;
extern PID_T PidSensorInstance;
extern Sensors_Manager_T SensorsManager;
extern SCP_Instance_T ScpInstance;

static void LF_CommandReset(const uint8_t *buffer, uint16_t size);
static void LF_CommandCalibrate(const uint8_t *buffer, uint16_t size);
static void LF_CommandSetPID(const uint8_t *buffer, uint16_t size);
static void LF_CommandGetSensorWeights(const uint8_t *buffer, uint16_t size);
static void LF_CommandSetSensorWeights(const uint8_t *buffer, uint16_t size);

const SCP_Command_T lineFollowerCommands[LINEFOLLOWER_COMMANDS_NUMBER] = {
    {0x0000, 0U, LF_CommandReset},
    {0x0001, 0U, LF_CommandCalibrate},

    {0x0100, 13U, LF_CommandSetPID},
    {0x0101, 12U, LF_CommandSetSensorWeights},

    {0x0201, 0U, LF_CommandGetSensorWeights},
};

static void LF_CommandReset(const uint8_t *buffer, uint16_t size)
{
    HAL_NVIC_SystemReset();
}

static void LF_CommandCalibrate(const uint8_t *buffer, uint16_t size)
{
    uint8_t response[2] = {0xA5U, 0xA5U};

    // Sensors_Calibrate(&SensorsManager);
    SCP_Transmit(&ScpInstance, response, sizeof(response));
}

static void LF_CommandSetPID(const uint8_t *buffer, uint16_t size)
{
    uint8_t response[2] = {0xA5U, 0xA5U};
    struct pidData
    {
        uint8_t pidType;
        float kp;
        float ki;
        float kd;
    } __attribute__((__packed__));

    const struct pidData *input = (const struct pidData *)buffer;

    switch (input->pidType)
    {
    case 0:
        NVM_Block.pidStgSensor.kp = input->kp;
        NVM_Block.pidStgSensor.ki = input->ki;
        NVM_Block.pidStgSensor.kd = input->kd;
        PID_Init(&PidSensorInstance, &NVM_Block.pidStgSensor);
        break;
    /* TODO: Handle other PID controllers after implementation
    case 1:
        NVM_Block.pidStgMotorLeft.kp = input->kp;
        NVM_Block.pidStgMotorLeft.ki = input->ki;
        NVM_Block.pidStgMotorLeft.kd = input->kd;
        PID_Init(&PidMotorLeftInstance, &NVM_Block.pidStgMotorLeft);
        break;
    case 2:
        NVM_Block.pidStgMotorRight.kp = input->kp;
        NVM_Block.pidStgMotorRight.ki = input->ki;
        NVM_Block.pidStgMotorRight.kd = input->kd;
        PID_Init(&PidMotorRightInstance, &NVM_Block.pidStgMotorRight);
        break;
    */
    default:
        break;
    }

    SCP_Transmit(&ScpInstance, response, sizeof(response));
}

static void LF_CommandGetSensorWeights(const uint8_t *buffer, uint16_t size)
{
    SCP_Transmit(&ScpInstance, NVM_Block.sensorWeights, sizeof(NVM_Block.sensorWeights));
}

static void LF_CommandSetSensorWeights(const uint8_t *buffer, uint16_t size)
{
    uint8_t response[2] = {0xA5U, 0xA5U};

    memcpy(NVM_Block.sensorWeights, buffer, size);
    Sensors_Config_Init(&hadc1, NVM_Block.sensorWeights);

    SCP_Transmit(&ScpInstance, response, sizeof(response));
}
