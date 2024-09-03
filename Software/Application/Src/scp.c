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
 * @param[in] buffer Pointer to the buffer used for UART reception.
 * @param[in] size Size of the buffer.
 * @param[in] huart Pointer to the UART handle.
 * @param[in] commands Array of SCP commands that can be processed.
 * @param[in] numCommands Number of commands in the array.
 * 
 * @return 
 * - 0 on success.
 * - -1 if fails.
 */
int SCP_Init(SCP_Instance_T *const scp, uint8_t *const buffer, const uint16_t size,
             UART_HandleTypeDef *const huart, SCP_Command_T *const commands, const size_t numCommands)
{
    if (!scp || !buffer || !huart || !commands || numCommands == 0U)
    {
        return -1;
    }

    scp->buffer = buffer;
    scp->size = size;
    scp->huart = huart;
    scp->commands = commands;
    scp->numCommands = numCommands;

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
