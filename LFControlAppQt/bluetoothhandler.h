#ifndef BLUETOOTHHANDLER_H
#define BLUETOOTHHANDLER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QTimer>
#include "command.h"
#include "nvmlayout.h"

class BluetoothHandler : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothHandler(QObject *parent = nullptr);

    void startDeviceDiscovery();
    void connectToDevice(const QBluetoothAddress &address);
    void disconnectFromDevice();
    bool isConnected() const;
    void sendCommand(Command command, const QByteArray &data);
    QList<QBluetoothDeviceInfo> discoveredDevices() const;

private slots:
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void handleDiscoveryFinished();
    void handleConnectionEstablished();
    void handleConnectionLost();
    void handleSocketReadyRead();
    void handleConnectionTimeout();
    void handleResponseTimeout();

signals:
    void deviceFound(const QBluetoothDeviceInfo &device);
    void discoveryFinished();
    void connectionEstablished();
    void connectionLost();
    void dataReceived(Command command, const QByteArray &data);

private:
    constexpr static int CONNECTION_TIMEOUT = 10000;
    constexpr static int RESPONSE_TIMEOUT = 1000;
    constexpr static size_t NVM_LAYOUT_SIZE = NVMLayout().size();

    const std::unordered_map<Command, qsizetype> commandResponseSize =
    {
        {Command::SetMode, 1},
        {Command::Reset, 0},
        {Command::Calibrate, 1},
        {Command::ReadNvmData, NVM_LAYOUT_SIZE},
        {Command::WriteNvmData, 1},
        {Command::SetDebugMode, 1},
        {Command::SetPID, 1},
        {Command::SetSensorWeights, 1},
        {Command::GetSensorWeights, 0}
    };

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *bluetoothSocket;
    QTimer *connectionTimer;
    QTimer *responseTimer;

    QByteArray dataBuffer;
    Command currentCommand;

    void processReceivedData();
};

#endif // BLUETOOTHHANDLER_H
