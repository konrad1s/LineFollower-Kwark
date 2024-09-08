#include "scp.h"
#include <string.h>
#include <stdio.h>

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
}

/**
 * @brief Enqueue a command into the SCP command queue.
 *
 * @param[in] scpQueue Pointer to the SCP command queue.
 * @param[in] command The command string to enqueue.
 * @param[in] size The size of the command string.
 */
void SCP_Dispatcher_Enqueue(SCP_DispatcherQueue_T *scpQueue, const char *command, uint16_t size)
{
    if ((scpQueue->count < SCP_COMMAND_QUEUE_SIZE) && (size < SCP_COMMAND_BUFFER_SIZE))
    {
        memcpy(scpQueue->commands[scpQueue->rear], command, size);
        /* Ensure null termination */
        scpQueue->commands[scpQueue->rear][size] = '\0';

        scpQueue->rear = (scpQueue->rear + 1U) % SCP_COMMAND_QUEUE_SIZE;
        scpQueue->count++;
    }
    else
    {
        /* TODO: Handle error */
    }
}

/**
 * @brief Processes the next command in the SCP queue.
 *
 * @param[in] scp Pointer to the SCP instance.
 */
void SCP_Dispatcher_Process(SCP_Instance_T *scp)
{
    if (scp->queue.count > 0U)
    {
        char *command = scp->queue.commands[scp->queue.front];
        char *args = strchr(command, ' ');

        if (args)
        {
            *args = '\0';
            args++;
        }

        for (size_t i = 0U; i < scp->numCommands; i++)
        {
            if (strcmp(command, scp->commands[i].command) == 0)
            {
                scp->commands[i].function(args);
                break;
            }
        }

        scp->queue.front = (scp->queue.front + 1U) % SCP_COMMAND_QUEUE_SIZE;
        scp->queue.count--;
    }
}
