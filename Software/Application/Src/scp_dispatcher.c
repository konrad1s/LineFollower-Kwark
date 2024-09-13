#include "scp.h"
#include <string.h>
#include <stdio.h>

#define SCP_COMMAND_HEADER_SIZE 2U
#define SCP_COMMAND_ID_SIZE     2U

/**
 * @brief Initializes the SCP command queue.
 *
 * @param[in] scpQueue Pointer to the SCP command queue.
 */
void SCP_Dispatcher_Init(SCP_DispatcherQueue_T *scpQueue)
{
    scpQueue->front = 0U;
    scpQueue->rear = 0U;
    scpQueue->count = 0U;
    memset(scpQueue->buffer, 0, SCP_GLOBAL_BUFFER_SIZE);
}

/**
 * @brief Enqueue a command into the global circular buffer.
 *
 * @param[in] scpQueue Pointer to the SCP command queue.
 * @param[in] command The byte array of the command to enqueue.
 * @param[in] size The size of the command data.
 */
void SCP_Dispatcher_Enqueue(SCP_DispatcherQueue_T *scpQueue, const uint8_t *command, uint16_t size)
{
    uint16_t total_size = size + SCP_COMMAND_HEADER_SIZE;

    if (scpQueue->count + total_size <= SCP_GLOBAL_BUFFER_SIZE)
    {
        scpQueue->buffer[scpQueue->rear] = (size >> 8U) & 0xFFU; 
        scpQueue->buffer[(scpQueue->rear + 1U) % SCP_GLOBAL_BUFFER_SIZE] = size & 0xFFU;

        for (uint16_t i = 0U; i < size; i++)
        {
            scpQueue->buffer[(scpQueue->rear + SCP_COMMAND_HEADER_SIZE + i) % SCP_GLOBAL_BUFFER_SIZE] = command[i];
        }

        scpQueue->rear = (scpQueue->rear + total_size) % SCP_GLOBAL_BUFFER_SIZE;
        scpQueue->count += total_size;
    }
    else
    {
        // TODO: Handle buffer overflow (e.g., log an error or trigger an alert)
    }
}

/**
 * @brief Processes the next command in the global buffer.
 *
 * @param[in] scp Pointer to the SCP instance.
 */
void SCP_Dispatcher_Process(SCP_Instance_T *scp, void *context)
{
    SCP_DispatcherQueue_T *scpQueue = &scp->queue;

    if (scpQueue->count > 0U)
    {
        const uint16_t size = (scpQueue->buffer[scpQueue->front] << 8U) |
                              scpQueue->buffer[(scpQueue->front + 1U) % SCP_GLOBAL_BUFFER_SIZE];

        /* Get the command data (starting after the 2-byte length header) */
        uint8_t *command = &scpQueue->buffer[(scpQueue->front + SCP_COMMAND_HEADER_SIZE) % SCP_GLOBAL_BUFFER_SIZE];

        for (size_t i = 0U; i < scp->numCommands; i++)
        {
            const uint16_t id = (command[0] << 8U) | command[1];
            const uint16_t cmdSize = size - SCP_COMMAND_ID_SIZE;

            if ((id == scp->commands[i].id) && (cmdSize == scp->commands[i].size))
            {
                scp->commands[i].function(context, &command[SCP_COMMAND_ID_SIZE], size - SCP_COMMAND_ID_SIZE);
                break;
            }
        }

        scpQueue->front = (scpQueue->front + size + SCP_COMMAND_HEADER_SIZE) % SCP_GLOBAL_BUFFER_SIZE;
        scpQueue->count -= (size + SCP_COMMAND_HEADER_SIZE);
    }
}
