/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "linefollower_config.h"
#include "linefollower_commands.h"
#include "lf_main.h"
#include "sensors.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define LF_COMMAND_MODE_START   0x00U
#define LF_COMMAND_MODE_STOP    0x01U

#define LF_COMMAND_ENTER_BOOT_FLAG 0xDEADBEEF

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static void LF_SetMode(const SCP_Packet *const packet, void *context);
static void LF_CommandReset(const SCP_Packet *const packet, void *context);
static void LF_CommandCalibrate(const SCP_Packet *const packet, void *context);
static void LF_ReadNvmData(const SCP_Packet *const packet, void *context);
static void LF_WriteNvmData(const SCP_Packet *const packet, void *context);
static void LF_SetDebugMode(const SCP_Packet *const packet, void *context);
static void LF_GetSession(const SCP_Packet *const packet, void *context);
static void LF_EnterBootloader(const SCP_Packet *const packet, void *context);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/
extern NVM_Layout_T NVM_Block;
extern PID_Instance_T PidSensorInstance;
extern SCP_Instance_T ScpInstance;

const SCP_Command_T lineFollowerCommands[LINEFOLLOWER_COMMANDS_NUMBER] = {
    {LF_CMD_SET_MODE,       1U,                     LF_SetMode},
    {LF_CMD_RESET,          0U,                     LF_CommandReset},
    {LF_CMD_CALIBRATE,      0U,                     LF_CommandCalibrate},
    {LF_CMD_READ_NVM_DATA,  0U,                     LF_ReadNvmData},
    {LF_CMD_WRITE_NVM_DATA, sizeof(NVM_Layout_T),   LF_WriteNvmData},
    {LF_CMD_SET_DEBUG_MODE, 1U,                     LF_SetDebugMode},
    {LF_CMD_GET_SESSION,    0U,                     LF_GetSession},

    {LF_CMD_ENTER_BOOTLOADER, 0U, LF_EnterBootloader},
};

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
static inline void LF_CommandTransmitResponse(LineFollower_T *me, uint16_t command_id, const void *responseData, uint16_t responseSize)
{
    SCP_Transmit(&me->scpInstance, command_id, responseData, responseSize);
}

static void LF_SetMode(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    if (packet->data[0] == LF_COMMAND_MODE_START)
    {
        LF_SendSignal(me, LF_SIG_START);
    }
    else if (packet->data[0] == LF_COMMAND_MODE_STOP)
    {
        LF_SendSignal(me, LF_SIG_STOP);
    }

    LF_CommandTransmitResponse(me, LF_CMD_SET_MODE, NULL, 0);
}

static void LF_CommandReset(const SCP_Packet *const packet, void *context)
{
    HAL_NVIC_SystemReset();
}

static void LF_CommandCalibrate(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    LF_SendSignal(me, LF_SIG_CALIBRATE);
}

static void LF_ReadNvmData(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    LF_CommandTransmitResponse(me, LF_CMD_READ_NVM_DATA, me->nvmBlock, sizeof(NVM_Layout_T));
}

static void LF_WriteNvmData(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    memcpy(me->nvmBlock, packet->data, packet->header.size);
    NVM_Write(&me->nvmInstance);

    LF_CommandTransmitResponse(me, LF_CMD_WRITE_NVM_DATA, NULL, 0);
}

static void LF_SetDebugMode(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;
    HAL_StatusTypeDef status;
    enum
    {
        LF_DEBUG_MODE_OFF,
        LF_DEBUG_MODE_ON
    } debugMode = packet->data[0];

    me->isDebugMode = (debugMode == LF_DEBUG_MODE_ON);

    if (me->isDebugMode)
    {
        status = HAL_TIM_Base_Start_IT(me->debugModeTimer);
    }
    else
    {
        status = HAL_TIM_Base_Stop_IT(me->debugModeTimer);
    }

    if (status == HAL_OK)
    {
        LF_CommandTransmitResponse(me, LF_CMD_SET_DEBUG_MODE, NULL, 0);
    }
}

static void LF_GetSession(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;
    const uint8_t responseData[] = "Application";

    SCP_Transmit(&me->scpInstance, LF_CMD_GET_SESSION, responseData, sizeof(responseData) - 1);
};

static void LF_EnterBootloader(const SCP_Packet *const packet, void *context)
{
    LineFollower_T *const me = (LineFollower_T *const )context;

    *me->bootFlags = LF_COMMAND_ENTER_BOOT_FLAG;
    HAL_NVIC_SystemReset();
}
