#include "scp.h"
#include <string.h>

typedef struct
{
    size_t numInstances;
    SCP_Instance_T *scpInstances[SCP_MAX_HUART_INSTANCES];
} SCP_Manager_T;

static SCP_Manager_T scpManager = {.numInstances = 0U};

/**
 * @brief Registers a new SCP instance for UART communication.
 *
 * @param[in] scp Pointer to the SCP instance.
 * 
 * @return 
 * - 0 on success.
 * - -1 if fails.
 */
static int SCP_RegisterInstance(SCP_Instance_T *const scp)
{
    if (scpManager.numInstances >= SCP_MAX_HUART_INSTANCES)
    {
        return -1;
    }

    if (HAL_UARTEx_ReceiveToIdle_DMA(scp->huart, scp->buffer, scp->size) != HAL_OK)
    {
        return -1;
    }

    scpManager.scpInstances[scpManager.numInstances++] = scp;

    return 0;
}

/**
 * @brief Processes a command received via UART.
 * 
 * @param[in] scp Pointer to the SCP instance containing the command to process.
 */
static void SCP_ProcessCommand(SCP_Instance_T *const scp)
{
    if (!scp || !scp->commands || scp->numCommands == 0U)
    {
        return;
    }

    /* Parse the command and arguments from the SCP buffer */
    char *args = strchr((char *)scp->buffer, ' ');

    if (args)
    {
        /* Null-terminate the command part */
        *args = '\0';
        args++;
    }

    for (size_t i = 0U; i < scp->numCommands; i++)
    {
        if (strcmp((char *)scp->buffer, scp->commands[i].command) == 0)
        {
            scp->commands[i].function(args);
            return; 
        }
    }

    /* If no matching command is found, the command is ignored */
}

/**
 * @brief Initializes an SCP instance for UART communication.
 *
 * @param[in,out] scp Pointer to the SCP instance to initialize.
 * @param[in] config Pointer to the configuration structure for the SCP instance.
 * 
 * @return 
 * - 0 on success.
 * - -1 if fails.
 */
int SCP_Init(SCP_Instance_T *const scp, const SCP_Config_T *const config)
{
    if (!scp || !config || !config->buffer || !config->huart || !config->commands || config->numCommands == 0U)
    {
        return -1;
    }

    scp->buffer = config->buffer;
    scp->size = config->size;
    scp->huart = config->huart;
    scp->commands = config->commands;
    scp->numCommands = config->numCommands;

    if (SCP_RegisterInstance(scp) != 0)
    {
        return -1; 
    }

    return 0;
}

/**
 * @brief UART receive event callback for handling incoming data.
 *
 * @param[in] huart Pointer to the UART handle associated with the received data.
 * @param[in] Size Size of the received data.
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    for (size_t i = 0U; i < scpManager.numInstances; i++)
    {
        SCP_Instance_T *scp = scpManager.scpInstances[i];
        if (scp && scp->huart == huart)
        {
            if (Size < scp->size)
            {
                scp->buffer[Size] = '\0';
            }
            else
            {
                /* Buffer overflow occurred, ignore the command */
                return;
            }

            SCP_ProcessCommand(scp);

            HAL_UARTEx_ReceiveToIdle_DMA(scp->huart, scp->buffer, scp->size);
            return;
        }
    }
}
