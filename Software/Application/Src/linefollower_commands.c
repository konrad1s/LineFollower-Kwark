#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "lf_main.h"
#include "sensors.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LF_COMMAND_MODE_START   0x00U
#define LF_COMMAND_MODE_STOP    0x01U

extern NVM_Layout_T NVM_Block;
extern PID_Instance_T PidSensorInstance;
extern SCP_Instance_T ScpInstance;

static void LF_SetMode(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandReset(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandCalibrate(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandSetPID(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandGetSensorWeights(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandSetSensorWeights(void *context, const uint8_t *buffer, uint16_t size);

const SCP_Command_T lineFollowerCommands[LINEFOLLOWER_COMMANDS_NUMBER] = {
    {0x0000, 1U, LF_SetMode},
    {0x0001, 0U, LF_CommandReset},
    {0x0002, 0U, LF_CommandCalibrate},

    {0x0100, 13U, LF_CommandSetPID},
    {0x0101, 12U, LF_CommandSetSensorWeights},

    {0x0201, 0U, LF_CommandGetSensorWeights},
};

static void LF_SetMode(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    if (buffer[0] == LF_COMMAND_MODE_START)
    {
        LF_SendSignal(me, START_SIG);
    }
    else if (buffer[0] == LF_COMMAND_MODE_STOP)
    {
        LF_SendSignal(me, STOP_SIG);
    }
}

static void LF_CommandReset(void *context, const uint8_t *buffer, uint16_t size)
{
    HAL_NVIC_SystemReset();
}

static void LF_CommandCalibrate(void *context, const uint8_t *buffer, uint16_t size)
{
    (void)buffer;
    (void)size;

    LineFollower_T *const me = (LineFollower_T *const )context;

    LF_SendSignal(me, CALIBRATE_SIG);
}

static void LF_CommandSetPID(void *context, const uint8_t *buffer, uint16_t size)
{
    struct pidData
    {
        uint8_t pidType;
        float kp;
        float ki;
        float kd;
    } __attribute__((__packed__));

    LineFollower_T *const me = (LineFollower_T *const )context;
    const struct pidData *input = (const struct pidData *)buffer;

    switch (input->pidType)
    {
    case 0:
        me->nvmBlock->pidStgSensor.kp = input->kp;
        me->nvmBlock->pidStgSensor.ki = input->ki;
        me->nvmBlock->pidStgSensor.kd = input->kd;
        PID_Init(&me->pidSensorInstance);
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
}

static void LF_CommandGetSensorWeights(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    SCP_Transmit(&me->scpInstance, me->nvmBlock->sensorWeights, sizeof(me->nvmBlock->sensorWeights));
}

static void LF_CommandSetSensorWeights(void *context, const uint8_t *buffer, uint16_t size)
{
    // memcpy(NVM_Block.sensorWeights, buffer, size);
    // // Sensors_Config_Init(&hadc1, NVM_Block.sensorWeights);

    // SCP_Transmit(&ScpInstance, response, sizeof(response));
}
