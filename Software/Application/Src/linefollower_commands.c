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

enum LF_Commands
{
    LF_CMD_SET_MODE         = 0x0000,
    LF_CMD_RESET            = 0x0002,
    LF_CMD_CALIBRATE        = 0x0004,
    LF_CMD_READ_NVM_DATA    = 0x0006,
    LF_CMD_WRITE_NVM_DATA   = 0x0008,
    LF_CMD_GET_DEBUG_DATA   = 0x00010,

    LF_CMD_SET_PID              = 0x0100,
    LF_CMD_SET_SENSOR_WEIGHTS   = 0x0102,

    LF_CMD_GET_SENSOR_WEIGHTS = 0x0200,
};

typedef struct
{
    uint16_t sensorsValues[SENSORS_NUMBER];
} Lf_CommandDebugData_T;

extern NVM_Layout_T NVM_Block;
extern PID_Instance_T PidSensorInstance;
extern SCP_Instance_T ScpInstance;

static void LF_SetMode(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandReset(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandCalibrate(void *context, const uint8_t *buffer, uint16_t size);
static void LF_ReadNvmData(void *context, const uint8_t *buffer, uint16_t size);
static void LF_WriteNvmData(void *context, const uint8_t *buffer, uint16_t size);
static void LF_GetDebugData(void *context, const uint8_t *buffer, uint16_t size);

static void LF_CommandSetPID(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandGetSensorWeights(void *context, const uint8_t *buffer, uint16_t size);
static void LF_CommandSetSensorWeights(void *context, const uint8_t *buffer, uint16_t size);

const SCP_Command_T lineFollowerCommands[LINEFOLLOWER_COMMANDS_NUMBER] = {
    {LF_CMD_SET_MODE,       1U,                     LF_SetMode},
    {LF_CMD_RESET,          0U,                     LF_CommandReset},
    {LF_CMD_CALIBRATE,      0U,                     LF_CommandCalibrate},
    {LF_CMD_READ_NVM_DATA,  0U,                     LF_ReadNvmData},
    {LF_CMD_WRITE_NVM_DATA, sizeof(NVM_Layout_T),   LF_WriteNvmData},
    {LF_CMD_GET_DEBUG_DATA, 1U,                     LF_GetDebugData},

    {LF_CMD_SET_PID,            13U, LF_CommandSetPID},
    {LF_CMD_SET_SENSOR_WEIGHTS, 12U, LF_CommandSetSensorWeights},

    {LF_CMD_GET_SENSOR_WEIGHTS, 0U, LF_CommandGetSensorWeights},
};

static void LF_CommandTransmitResponse(LineFollower_T *me, uint16_t command_id, const void *responseData, uint16_t responseSize)
{
    uint16_t responseCmd = command_id + 1U;

    SCP_Transmit(&me->scpInstance, &responseCmd, sizeof(responseCmd));

    if (responseData != NULL && responseSize > 0U)
    {
        SCP_Transmit(&me->scpInstance, responseData, responseSize);
    }
}

static void LF_SetMode(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    if (buffer[0] == LF_COMMAND_MODE_START)
    {
        LF_SendSignal(me, LF_SIG_START);
    }
    else if (buffer[0] == LF_COMMAND_MODE_STOP)
    {
        LF_SendSignal(me, LF_SIG_STOP);
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

    LF_SendSignal(me, LF_SIG_CALIBRATE);
}

static void LF_ReadNvmData(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    LF_CommandTransmitResponse(me, LF_CMD_READ_NVM_DATA, me->nvmBlock, sizeof(NVM_Layout_T));
}

static void LF_WriteNvmData(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    memcpy(me->nvmBlock, buffer, size);
    NVM_Write(&me->nvmInstance);

    LF_CommandTransmitResponse(me, LF_CMD_WRITE_NVM_DATA, NULL, 0);
}

static void LF_GetDebugData(void *context, const uint8_t *buffer, uint16_t size)
{
    LineFollower_T *const me = (LineFollower_T *const )context;
    Lf_CommandDebugData_T debugData;

    Sensors_GetRawData(debugData.sensorsValues);

    LF_CommandTransmitResponse(me, LF_CMD_GET_DEBUG_DATA, &debugData, sizeof(debugData));
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

    LF_CommandTransmitResponse(me, LF_CMD_GET_SENSOR_WEIGHTS,
                               me->nvmBlock->sensors.weights, sizeof(me->nvmBlock->sensors.weights));
}

static void LF_CommandSetSensorWeights(void *context, const uint8_t *buffer, uint16_t size)
{
    // memcpy(NVM_Block.sensorWeights, buffer, size);
    // // Sensors_Config_Init(&hadc1, NVM_Block.sensorWeights);

    // SCP_Transmit(&ScpInstance, response, sizeof(response));
}
