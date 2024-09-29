#ifndef SCP_H
#define SCP_H

#include <QObject>
#include <QByteArray>
#include <optional>
#include "command.h"
#include "nvmlayout.h"
#include "debugdata.h"

class SCP : public QObject
{
    Q_OBJECT
public:
    explicit SCP(QObject *parent = nullptr);

    void sendCommand(Command command, const QByteArray &data = QByteArray());
    void receiveData(const QByteArray &data);

signals:
    void packetReadyToSend(const QByteArray &packet);
    void commandReceived(Command command, const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    enum class PacketState
    {
        Idle,
        GotStart,
        GotCrc,
        GotId,
        GotSize,
        GettingData
    };

    constexpr static uint8_t START_BYTE = 0x7E;
    constexpr static uint16_t CRC_POLYNOMIAL = 0x8408;
    constexpr static qsizetype ACK_RESPONSE_SIZE = 2;
    constexpr static qsizetype NVM_LAYOUT_SIZE = NVMLayout().size();
    constexpr static qsizetype DEBUG_DATA_SIZE = DebugData().size();

    const std::unordered_map<Command, qsizetype> commandDataSize = {
        {Command::SetMode,          ACK_RESPONSE_SIZE},
        {Command::Reset,            0},
        {Command::Calibrate,        ACK_RESPONSE_SIZE},
        {Command::ReadNvmData,      NVM_LAYOUT_SIZE},
        {Command::WriteNvmData,     ACK_RESPONSE_SIZE},
        {Command::GetDebugData,     DEBUG_DATA_SIZE},
        {Command::SetPID,           ACK_RESPONSE_SIZE},
        {Command::SetSensorWeights, ACK_RESPONSE_SIZE},
        {Command::GetSensorWeights, 0}
    };

    PacketState packetState;
    
#pragma pack(push, 1)
    struct PacketHeader
    {
        uint8_t start;
        uint16_t crc;
        uint16_t id;
        uint8_t size;
    };
#pragma pack(pop)

    PacketHeader currentHeader;
    QByteArray packetData;
    int dataBytesReceived;
    QByteArray dataBuffer;

    uint16_t calculateCRC(const uint8_t *data, uint16_t size, uint16_t init = 0x1D0F);
    void resetPacketReception();
    bool validateAndProcessPacket();
    
    std::optional<uint16_t> extractUInt16FromBuffer(QByteArray &buffer);
    std::optional<uint8_t> extractUInt8FromBuffer(QByteArray &buffer);
};

#endif // SCP_H
