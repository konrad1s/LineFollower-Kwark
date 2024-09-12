#include "scp.h"
#include <string.h>

#define SCP_ErrorHandler(scp)                             \
    do                                                    \
    {                                                     \
        if (scp->errorHandler)                            \
        {                                                 \
            scp->errorHandler((const char *)scp->buffer); \
        }                                                 \
    } while (0)

typedef struct
{
    size_t numInstances;
    SCP_Instance_T *scpInstances[SCP_MAX_HUART_INSTANCES];
} SCP_Manager_T;

extern void SCP_Dispatcher_Init(SCP_DispatcherQueue_T *scpQueue);
extern void SCP_Dispatcher_Enqueue(SCP_DispatcherQueue_T *scpQueue, const uint8_t *command, uint16_t size);
extern void SCP_Dispatcher_Process(SCP_Instance_T *scp);

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
 * @brief Initializes an SCP instance for UART communication.
 *
 * @param[in,out] scp Pointer to the SCP instance to initialize.
 * @param[in] config Pointer to the configuration structure for the SCP instance.
 * 
 * @return 
 * - 0 on success.
 * - -1 if fails.
 */
int SCP_Init(SCP_Instance_T *const scp)
{
    if (NULL == scp || NULL == scp->buffer || NULL == scp->huart || NULL == scp->commands || scp->numCommands == 0)
    {
        return -1;
    }

    if (SCP_RegisterInstance(scp) != 0)
    {
        return -1; 
    }

    SCP_Dispatcher_Init(&scp->queue);

    return 0;
}

/**
 * @brief Processes all registered SCP instances.
 */
void SCP_Process(void)
{
    for (size_t i = 0U; i < scpManager.numInstances; i++)
    {
        SCP_Instance_T *scp = scpManager.scpInstances[i];

        if (scp)
        {
            SCP_Dispatcher_Process(scp);
        }
    }
}

/**
 * @brief Transmits data over UART
 *
 * @param[in] scp Pointer to the SCP instance.
 * @param[in] data Pointer to the data to be transmitted.
 * @param[in] size Size of the data to be transmitted.
 * 
 * @return 
 * - 0 on success.
 * - -1 on failure.
 */
int SCP_Transmit(SCP_Instance_T *const scp, const uint8_t *data, uint16_t size)
{
    if (!scp || !data || size == 0)
    {
        return -1;
    }

    /* TODO: Currently blocking, consider using non-blocking transmission */
    if (HAL_UART_Transmit(scp->huart, (uint8_t *)data, size, HAL_MAX_DELAY) != HAL_OK)
    {
        SCP_ErrorHandler(scp);
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
            SCP_Dispatcher_Enqueue(&scp->queue, scp->buffer, Size);
            HAL_UARTEx_ReceiveToIdle_DMA(scp->huart, scp->buffer, scp->size);
            return;
        }
    }
}
