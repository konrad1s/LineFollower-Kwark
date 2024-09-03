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
} SCP_Instance_T;

int SCP_Init(SCP_Instance_T *const scp, uint8_t *const buffer, const uint16_t size,
             UART_HandleTypeDef *const huart, SCP_Command_T *const commands, const size_t numCommands);

#endif /* __SCP__H__ */
