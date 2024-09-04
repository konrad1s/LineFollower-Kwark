#ifndef __SCP__H__
#define __SCP__H__

#include <stdint.h>
#include <stddef.h>
#include "usart.h"

#define SCP_MAX_HUART_INSTANCES 1U

typedef struct
{
    const char *command;
    void (*function)(const char *args);
} SCP_Command_T;

typedef struct
{
    uint8_t *buffer;
    uint16_t size;
    UART_HandleTypeDef *huart;
    SCP_Command_T *commands;
    size_t numCommands;
    void (*errorHandler)(const char *command);
} SCP_Common_T;

typedef SCP_Common_T SCP_Config_T;
typedef SCP_Common_T SCP_Instance_T;

int SCP_Init(SCP_Instance_T *const scp, const SCP_Config_T *const config);

#endif /* __SCP__H__ */
