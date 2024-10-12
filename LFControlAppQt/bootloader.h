#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QObject>
#include <QFile>
#include <QByteArray>
#include <QTimer>
#include <QMap>
#include <QDataStream>
#include "command.h"
#include "bluetoothhandler.h"

class Bootloader : public QObject
{
    Q_OBJECT
public:
    explicit Bootloader(BluetoothHandler *bluetoothHandler, QObject *parent = nullptr);

    void enterBootloader();
    void getBootloaderVersion();
    bool selectFirmwareFile(const QString &filePath);
    void loadKey(const QByteArray &key);
    void eraseApplication();
    void flashFirmware();
    void validateApplication();
    void jumpToApplication();

signals:
    void bootloaderVersionReceived(const QString &version);
    void progressUpdated(int progress);
    void bootloaderMessage(const QString &message);
    void errorOccurred(const QString &error);

private slots:
    void handleDataReceived(Command command, const QByteArray &data);
    bool parseHexLine(const QString &line);

private:
    enum class iHexRecordType
    {
        Data = 0,
        EndOfFile = 1,
        ExtendedSegmentAddress = 2,
        StartSegmentAddress = 3,
        ExtendedLinearAddress = 4,
        StartLinearAddress = 5
    };

    static const int FLASH_BLOCK_SIZE = 64;
    static const quint32 APP_START_ADDRESS = 0x0800C000U;
    static const quint32 APP_END_ADDRESS = 0x0805FFFFU;
    static const quint32 CRC_SIZE = 4U;

    BluetoothHandler *bluetoothHandler;
    QFile firmwareFile;
    QMap<quint32, quint8> firmwareMap;
    QMap<quint32, quint8>::const_iterator firmwareIterator;
    quint32 baseAddress;
    quint32 totalBytesToFlash;
    quint32 bytesSent;
    quint32 crcValue;

    quint32 calculateCRC32();
    void sendNextFirmwareChunk();
    void prepareFirmwareData();
};

#endif // BOOTLOADER_H
