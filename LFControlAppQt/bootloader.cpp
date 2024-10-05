#include "bootloader.h"
#include <QTextStream>
#include <QDataStream>

Bootloader::Bootloader(BluetoothHandler *bluetoothHandler, QObject *parent)
    : QObject(parent), bluetoothHandler(bluetoothHandler), currentOffset(0), baseAddress(0)
{
    connect(bluetoothHandler, &BluetoothHandler::dataReceived, this, &Bootloader::handleDataReceived);
}

void Bootloader::enterBootloader()
{
    bluetoothHandler->sendCommand(Command::BootStartDownload, QByteArray());
    emit bootloaderMessage("Enter bootloader command sent.");
}

void Bootloader::getBootloaderVersion()
{
    bluetoothHandler->sendCommand(Command::BootGetVersion, QByteArray());
    emit bootloaderMessage("Get bootloader version command sent.");
}

bool Bootloader::selectFirmwareFile(const QString &filePath)
{
    firmwareFile.setFileName(filePath);
    if (!firmwareFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit errorOccurred("Failed to open firmware file.");
        return false;
    }

    firmwareMap.clear();
    baseAddress = 0;

    QTextStream in(&firmwareFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (!parseHexLine(line))
        {
            emit errorOccurred("Failed to parse hex file.");
            firmwareFile.close();
            return false;
        }
    }
    firmwareFile.close();

    if (firmwareMap.isEmpty())
    {
        emit errorOccurred("No firmware data loaded.");
        return false;
    }

    currentOffset = 0;
    emit bootloaderMessage("Firmware file loaded successfully.");
    return true;
}

bool Bootloader::parseHexLine(const QString &line)
{
    bool ok;

    /* Intel HEX record format: :llaaaatt[dd...]cc */
    if (line.isEmpty() || !line.startsWith(":"))
        return false;

    quint8 recordLength = line.mid(1, 2).toUInt(&ok, 16);
    if (!ok)
        return false;

    quint16 address = line.mid(3, 4).toUInt(&ok, 16);
    if (!ok)
        return false;

    quint8 recordType = line.mid(7, 2).toUInt(&ok, 16);
    if (!ok)
        return false;

    QString dataString = line.mid(9, recordLength * 2);
    QByteArray dataBytes = QByteArray::fromHex(dataString.toLatin1());

    quint8 checksum = line.mid(9 + recordLength * 2, 2).toUInt(&ok, 16);
    if (!ok)
        return false;

    /* Verify checksum */
    quint8 calculatedChecksum = recordLength + (address >> 8) + (address & 0xFF) + recordType;
    for (const auto &byte : dataBytes)
        calculatedChecksum += static_cast<quint8>(byte);
    calculatedChecksum = (~calculatedChecksum + 1) & 0xFF;

    if (calculatedChecksum != checksum)
        return false;

    switch (static_cast<iHexRecordType>(recordType))
    {
    case iHexRecordType::Data:
        for (int i = 0; i < dataBytes.size(); ++i)
        {
            quint32 addr = baseAddress + address + i;
            firmwareMap[addr] = static_cast<quint8>(dataBytes[i]);
        }
        break;
    case iHexRecordType::EndOfFile:
        /* Do nothing */
        break;
    case iHexRecordType::ExtendedLinearAddress:
    {
        if (recordLength != 2)
            return false;
        quint16 highAddress = (static_cast<quint8>(dataBytes[0]) << 8) | static_cast<quint8>(dataBytes[1]);
        baseAddress = static_cast<quint32>(highAddress) << 16;
    }
    break;
    default:
        /* Unsupported or unnecessary record types - can be ignored */
        break;
    }

    return true;
}

void Bootloader::loadKey(const QByteArray &key)
{
    bluetoothHandler->sendCommand(Command::BootFlashMac, key);
    emit bootloaderMessage("Load key command sent.");
}

void Bootloader::eraseApplication()
{
    bluetoothHandler->sendCommand(Command::BootEraseApp, QByteArray());
    emit bootloaderMessage("Erase application command sent.");
}

void Bootloader::flashFirmware()
{
    if (firmwareMap.isEmpty())
    {
        emit errorOccurred("No firmware data loaded.");
        return;
    }

    firmwareIterator = firmwareMap.constBegin();
    totalBytes = firmwareMap.size();
    bytesSent = 0;
    sendNextFirmwareChunk();
}

void Bootloader::sendNextFirmwareChunk()
{
    if (firmwareIterator == firmwareMap.constEnd())
    {
        emit bootloaderMessage("Firmware flashing completed.");
        emit progressUpdated(100);
        return;
    }

    QByteArray chunkData;
    quint32 startAddress = firmwareIterator.key();

    int bytesInChunk = 0;
    quint32 currentAddress = startAddress;

    while (firmwareIterator != firmwareMap.constEnd() && bytesInChunk < FLASH_BLOCK_SIZE)
    {
        if (firmwareIterator.key() != currentAddress)
        {
            /* Non-contiguous address, start a new chunk */
            break;
        }

        chunkData.append(firmwareIterator.value());
        ++firmwareIterator;
        ++currentAddress;
        ++bytesInChunk;
    }

    QByteArray packetData;
    QDataStream out(&packetData, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    out << startAddress;
    out << static_cast<quint16>(bytesInChunk);
    packetData.append(chunkData);

    bluetoothHandler->sendCommand(Command::BootFlashData, packetData);

    bytesSent += bytesInChunk;

    int progress = (bytesSent * 100) / totalBytes;
    emit progressUpdated(progress);
    emit bootloaderMessage(QString("Flashing firmware: %1%").arg(progress));
}

void Bootloader::validateApplication()
{
    bluetoothHandler->sendCommand(Command::BootValidateApp, QByteArray());
    emit bootloaderMessage("Validate application command sent.");
}

void Bootloader::jumpToApplication()
{
    bluetoothHandler->sendCommand(Command::BootJumpToApp, QByteArray());
    emit bootloaderMessage("Jump to application command sent.");
}

void Bootloader::handleDataReceived(Command command, const QByteArray &data)
{
    switch (command)
    {
    case Command::BootFlashData:
    {
        sendNextFirmwareChunk();
        break;
    }
    default:
        break;
    }
}
