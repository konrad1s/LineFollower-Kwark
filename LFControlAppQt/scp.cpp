#include "scp.h"
#include <QDataStream>
#include <QIODevice>
#include <qdebug.h>

SCP::SCP(QObject *parent)
    : QObject(parent),
    packetState(PacketState::Idle),
    dataBytesReceived(0)
{
}

void SCP::sendCommand(Command command, const QByteArray &data)
{
    PacketHeader header = { START_BYTE, 0, static_cast<uint16_t>(command), static_cast<uint8_t>(data.size()) };

    QByteArray crcData;
    QDataStream crcStream(&crcData, QIODevice::WriteOnly);
    crcStream.setByteOrder(QDataStream::LittleEndian);
    crcStream << header.id << header.size;
    crcData.append(data);

    header.crc = calculateCRC(reinterpret_cast<const uint8_t*>(crcData.data()), crcData.size());

    QByteArray packet;
    QDataStream packetStream(&packet, QIODevice::WriteOnly);
    packetStream.setByteOrder(QDataStream::LittleEndian);
    packetStream << header.start << header.crc << header.id << header.size;
    packet.append(data);

    emit packetReadyToSend(packet);
}

void SCP::receiveData(const QByteArray &data)
{
    dataBuffer.append(data);

    while (true)
    {
        switch (packetState)
        {
        case PacketState::Idle:
        {
            if (auto byte = extractUInt8FromBuffer(dataBuffer); byte.has_value())
            {
                if (byte.value() == START_BYTE)
                {
                    packetState = PacketState::GotStart;
                }
            }
            else
            {
                /* Wait for more data */
                return;
            }
            break;
        }
        case PacketState::GotStart:
        {
            if (auto crc = extractUInt16FromBuffer(dataBuffer); crc.has_value())
            {
                currentHeader.crc = crc.value();
                packetState = PacketState::GotCrc;
            }
            else
            {
                /* Wait for more data */
                return;
            }
            break;
        }
        case PacketState::GotCrc:
        {
            if (auto id = extractUInt16FromBuffer(dataBuffer); id.has_value())
            {
                currentHeader.id = id.value();
                packetState = PacketState::GotId;
            }
            else
            {
                /* Wait for more data */
                return;
            }
            break;
        }
        case PacketState::GotId:
        {
            if (auto sizeByte = extractUInt8FromBuffer(dataBuffer); sizeByte.has_value())
            {
                currentHeader.size = sizeByte.value();
                dataBytesReceived = 0;
                packetData.clear();

                if (currentHeader.size > 0)
                {
                    packetState = PacketState::GettingData;
                }
                else
                {
                    if (validateAndProcessPacket())
                    {
                        resetPacketReception();
                    }
                    else
                    {
                        emit errorOccurred("CRC mismatch in received packet.");
                        resetPacketReception();
                    }
                }
            }
            else
            {
                /* Wait for more data */
                return;
            }
            break;
        }
        case PacketState::GettingData:
        {
            int bytesNeeded = currentHeader.size - dataBytesReceived;
            if (dataBuffer.size() >= bytesNeeded)
            {
                packetData.append(dataBuffer.left(bytesNeeded));
                dataBuffer.remove(0, bytesNeeded);
                dataBytesReceived += bytesNeeded;

                if (validateAndProcessPacket())
                {
                    resetPacketReception();
                }
                else
                {
                    emit errorOccurred("CRC mismatch in received packet.");
                    resetPacketReception();
                }
            }
            else
            {
                /* Wait for more data */
                return;
            }
            break;
        }
        default:
            resetPacketReception();
            break;
        }
    }
}

uint16_t SCP::calculateCRC(const uint8_t *data, uint16_t size, uint16_t init)
{
    uint16_t crc = init;
    for (uint16_t i = 0; i < size; ++i)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0x8408;
            else
                crc >>= 1;
        }
    }
    return crc;
}

void SCP::resetPacketReception()
{
    packetState = PacketState::Idle;
    currentHeader = PacketHeader();
    packetData.clear();
    dataBytesReceived = 0;
}

bool SCP::validateAndProcessPacket()
{
    const Command currentCommand = static_cast<Command>(currentHeader.id);

    if (!commandDataSize.contains(currentCommand))
    {
        emit errorOccurred("Received packet with unknown command.");
        return false;
    }
    if (currentHeader.size != commandDataSize.at(currentCommand))
    {
        emit errorOccurred("Received packet with invalid size.");
        return false;
    }

    QByteArray crcData;
    QDataStream crcStream(&crcData, QIODevice::WriteOnly);
    crcStream.setByteOrder(QDataStream::LittleEndian);
    crcStream << currentHeader.id << currentHeader.size;
    crcData.append(packetData);

    uint16_t calculatedCRC = calculateCRC(reinterpret_cast<const uint8_t*>(crcData.data()), crcData.size());

    if (calculatedCRC == currentHeader.crc)
    {
        Command command = static_cast<Command>(currentHeader.id);
        emit commandReceived(command, packetData);
        return true;
    }
    else
    {
        emit errorOccurred("CRC mismatch in received packet.");
        return false;
    }
}

std::optional<uint16_t> SCP::extractUInt16FromBuffer(QByteArray &buffer)
{
    if (buffer.size() < 2)
        return std::nullopt;

    uint16_t value = static_cast<uint8_t>(buffer.at(0)) | (static_cast<uint8_t>(buffer.at(1)) << 8);
    buffer.remove(0, 2);

    return value;
}

std::optional<uint8_t> SCP::extractUInt8FromBuffer(QByteArray &buffer)
{
    if (buffer.isEmpty())
        return std::nullopt;

    uint8_t value = static_cast<uint8_t>(buffer.at(0));
    buffer.remove(0, 1);

    return value;
}