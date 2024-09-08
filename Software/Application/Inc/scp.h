#ifndef __SCP__H__
#define __SCP__H__

#include <stdint.h>
#include <stddef.h>
#include "usart.h"
#include "scp_dispatcher.h"

#define SCP_MAX_HUART_INSTANCES 1U

typedef struct
{
    uint16_t id;
    void (*function)(const uint8_t *buffer, uint16_t size);
} SCP_Command_T;

typedef struct
{
    uint8_t *buffer;
    uint16_t size;
    UART_HandleTypeDef *huart;
    SCP_Command_T *commands;
    size_t numCommands;
    void (*errorHandler)(const char *command);

    SCP_DispatcherQueue_T queue;
} SCP_Instance_T;

typedef struct
{
    uint8_t *buffer;
    uint16_t size;
    UART_HandleTypeDef *huart;
    SCP_Command_T *commands;
    size_t numCommands;
    void (*errorHandler)(const char *command);
} SCP_Config_T;

int SCP_Init(SCP_Instance_T *const scp, const SCP_Config_T *const config);
void SCP_Process(void);

#endif /* __SCP__H__ */
