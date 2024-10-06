#include <string.h>
#include <stdio.h>
#include "scp.h"
#include "crc.h"
#include "cmsis_compiler.h"

#define SCP_Dispatcher_EnterCritical() __disable_irq()
#define SCP_Dispatcher_ExitCritical() __enable_irq()

/**
 * @brief Initializes the SCP command queue.
 *
 * @param[in] scpQueue Pointer to the SCP command queue.
 */
void SCP_Dispatcher_Init(SCP_DispatcherQueue_T *scpQueue)
{
    SCP_Dispatcher_EnterCritical();

    scpQueue->front = 0U;
    scpQueue->rear = 0U;
    scpQueue->count = 0U;
    memset(scpQueue->buffer, 0, SCP_GLOBAL_BUFFER_SIZE);

    SCP_Dispatcher_ExitCritical();
}

/**
 * @brief Enqueue a data into the global circular buffer.
 *
 * @param[in] scpQueue Pointer to the SCP queue.
 * @param[in] data The byte array to be enqueued.
 * @param[in] size The size of the data to be enqueued.
 */
void SCP_Dispatcher_Enqueue(SCP_DispatcherQueue_T *scpQueue, const uint8_t *data, uint16_t size)
{
    SCP_Dispatcher_EnterCritical();

    if (scpQueue->count < SCP_GLOBAL_BUFFER_SIZE)
    {
        for (uint16_t i = 0U; i < size; i++)
        {
            scpQueue->buffer[scpQueue->rear] = data[i];
            scpQueue->rear = (scpQueue->rear + 1U) % SCP_GLOBAL_BUFFER_SIZE;
        }
        scpQueue->count += size;
    }

    SCP_Dispatcher_ExitCritical();
}

/**
 * @brief Dequeue a data from the global circular buffer.
 *
 * @param[in] scpQueue Pointer to the SCP queue.
 * @param[out] data The byte array to be dequeued.
 *
 * @return 1 if the data was dequeued, 0 otherwise.
 */
uint8_t SCP_Dispatcher_Dequeue(SCP_DispatcherQueue_T *scpQueue, uint8_t *data)
{
    SCP_Dispatcher_EnterCritical();

    uint8_t retVal = 0U;

    if (scpQueue->count > 0U)
    {
        *data = scpQueue->buffer[scpQueue->front];
        scpQueue->front = (scpQueue->front + 1U) % SCP_GLOBAL_BUFFER_SIZE;
        scpQueue->count--;
        retVal = 1U;
    }

    SCP_Dispatcher_ExitCritical();

    return retVal;
}

/**
 * @brief Handles the received packet.
 * 
 * @param[in] scp Pointer to the SCP instance.
 * @param[in] context Pointer to the context, to be passed to the command handler.
 */
static void SCP_Dispatcher_HandlePacketReceived(SCP_Instance_T *scp, void *context)
{
    uint16_t crcDataSize = scp->receivedPacket.header.size + sizeof(scp->receivedPacket.header.id) + sizeof(scp->receivedPacket.header.size);
    uint16_t crc = CRC_CalculateCRC16((uint8_t *)&scp->receivedPacket.header.id, crcDataSize, SCP_PACKET_CRC_INIT);

    if (crc == scp->receivedPacket.header.crc)
    {
        for (size_t i = 0U; i < scp->numCommands; i++)
        {
            if (scp->receivedPacket.header.id == scp->commands[i].id)
            {
                scp->commands[i].handler(&scp->receivedPacket, context);
                break;
            }
        }
    }
}

/**
 * @brief Processes the next command in the global buffer.
 *
 * @param[in] scp Pointer to the SCP instance.
 * @param[in] context Pointer to the context, to be passed to the command handler.
 */
void SCP_Dispatcher_Process(SCP_Instance_T *scp, void *context)
{
    SCP_DispatcherQueue_T *scpQueue = &scp->queue;
    uint8_t byte = 0U;

    while (SCP_Dispatcher_Dequeue(scpQueue, &byte))
    {
        switch (scp->state)
        {
        case SCP_PACKET_STATE_IDLE:
            if (byte == SCP_PACKET_START)
            {
                scp->receivedPacket.header.start = byte;
                scp->receivedPacket.header.size = 0U;
                scp->state = SCP_PACKET_STATE_GOT_START;
            }
            break;
        case SCP_PACKET_STATE_GOT_START:
            scp->receivedPacket.header.crc = byte;
            scp->state = SCP_PACKET_STATE_GOT_CRC_LOW;
            break;
        case SCP_PACKET_STATE_GOT_CRC_LOW:
            scp->receivedPacket.header.crc |= (uint16_t)(byte << 8);
            scp->state = SCP_PACKET_STATE_GOT_CRC_HIGH;
            break;
        case SCP_PACKET_STATE_GOT_CRC_HIGH:
            scp->receivedPacket.header.id = byte;
            scp->state = SCP_PACKET_STATE_GOT_ID_LOW;
            break;
        case SCP_PACKET_STATE_GOT_ID_LOW:
            scp->receivedPacket.header.id |= (uint16_t)(byte << 8);
            scp->state = SCP_PACKET_STATE_GOT_SIZE;
            break;
        case SCP_PACKET_STATE_GOT_SIZE:
            scp->receivedPacket.header.size = byte;
            scp->dataBytesReceived = 0U;

            if (scp->receivedPacket.header.size > 0U)
            {
                scp->state = SCP_PACKET_STATE_GETTING_DATA;
            }
            else
            {
                scp->state = SCP_PACKET_STATE_PACKET_COMPLETE;
            }
            break;
        case SCP_PACKET_STATE_GETTING_DATA:
            scp->receivedPacket.data[scp->dataBytesReceived++] = byte;
            if (scp->dataBytesReceived == scp->receivedPacket.header.size)
            {
                scp->state = SCP_PACKET_STATE_PACKET_COMPLETE;
            }
            break;
        default:
            scp->state = SCP_PACKET_STATE_IDLE;
            break;
        }

        if (scp->state == SCP_PACKET_STATE_PACKET_COMPLETE)
        {
            SCP_Dispatcher_HandlePacketReceived(scp, context);
            scp->state = SCP_PACKET_STATE_IDLE;
        }
    }
}
