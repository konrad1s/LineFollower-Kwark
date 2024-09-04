#include "linefollower_config.h"
#include "usart.h"

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
};

/* ----------------------------------- SCP CONFIG ----------------------------------- */

static uint8_t scpBuffer[SCP_BUFFER_SIZE];

const SCP_Config_T ScpConfig = {
        .buffer = scpBuffer,
        .size = SCP_BUFFER_SIZE,
        .huart = &huart4,
        .commands = NULL,
        .numCommands = 0U,
        .errorHandler = NULL,
    };
