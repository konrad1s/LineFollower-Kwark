#ifndef __SCP__H__
#define __SCP__H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "usart.h"
#include "scp_dispatcher.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define SCP_MAX_HUART_INSTANCES 1U
#define SCP_PACKET_START        0x7EU
#define SCP_PACKET_CRC_INIT     0x1D0FU
#define SCP_PACKET_MAX_SIZE     2060U

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef uint16_t SCP_CommandId_T;

typedef struct __attribute__((packed))
{
    uint8_t start;
    uint16_t crc;
    SCP_CommandId_T id;
    uint8_t size;
} SCP_PacketHeader;

static_assert(6 == sizeof(SCP_PacketHeader), "6 != sizeof(SCP_PacketHeader)");

typedef struct __attribute__((packed))
{
    SCP_PacketHeader header;
    uint8_t data[SCP_PACKET_MAX_SIZE];
} SCP_Packet;

typedef void (*SCP_CommandHandler)(const SCP_Packet *const packet, void *context);

typedef struct
{
    uint16_t id;
    uint8_t size;
    SCP_CommandHandler handler;
} SCP_Command_T;

typedef enum 
{
    SCP_PACKET_STATE_IDLE,
    SCP_PACKET_STATE_GOT_START,
    SCP_PACKET_STATE_GOT_CRC_LOW,
    SCP_PACKET_STATE_GOT_CRC_HIGH,
    SCP_PACKET_STATE_GOT_ID_LOW,
    SCP_PACKET_STATE_GOT_SIZE,
    SCP_PACKET_STATE_GETTING_DATA,
    SCP_PACKET_STATE_PACKET_COMPLETE
} SCP_PacketState_T;

typedef struct
{
    uint8_t *buffer;
    uint16_t size;
    UART_HandleTypeDef *huart;
    const SCP_Command_T *commands;
    size_t numCommands;
    void (*errorHandler)(const char *command);

    SCP_Packet receivedPacket;
    uint8_t dataBytesReceived;
    SCP_PacketState_T state;
    SCP_DispatcherQueue_T queue;
} SCP_Instance_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
int SCP_Init(SCP_Instance_T *const scp);
void SCP_Process(void *context);
int SCP_Transmit(SCP_Instance_T *const scp, SCP_CommandId_T id, const void *data, uint16_t size);

#endif /* __SCP__H__ */
