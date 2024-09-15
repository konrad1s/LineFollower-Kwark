#ifndef BLUETOOTHHANDLER_H
#define BLUETOOTHHANDLER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QTimer>
#include "command.h"

class BluetoothHandler : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothHandler(QObject *parent = nullptr);

    void startDeviceDiscovery();
    void connectToDevice(const QBluetoothAddress &address);
    void disconnectFromDevice();
    bool isConnected() const;
    void sendCommand(Command command);
    QList<QBluetoothDeviceInfo> discoveredDevices() const;

private slots:
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void handleDiscoveryFinished();
    void handleConnectionEstablished();
    void handleConnectionLost();
    void handleSocketReadyRead();
    void handleConnectionTimeout();

signals:
    void deviceFound(const QBluetoothDeviceInfo &device);
    void discoveryFinished();
    void connectionEstablished();
    void connectionLost();
    void dataReceived(Command command, const QByteArray &data);

private:
    constexpr static int CONNECTION_TIMEOUT = 10000;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *bluetoothSocket;
    QTimer *connectionTimer;
    QTimer *responseTimer;

    QByteArray dataBuffer;
    qsizetype expectedResponseSize;
    Command currentCommand;

    void processReceivedData();
};

#endif // BLUETOOTHHANDLER_H
