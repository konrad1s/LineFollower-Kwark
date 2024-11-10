#ifndef BLUETOOTHHANDLER_H
#define BLUETOOTHHANDLER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QTimer>
#include "command.h"
#include "scp.h"

class BluetoothHandler : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothHandler(QObject *parent = nullptr);
    ~BluetoothHandler();

    void startDeviceDiscovery();
    void stopDeviceDiscovery();
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
    void handlePacketReadyToSend(const QByteArray &packet);
    void handleCommandReceived(Command command, const QByteArray &data);
    void handleProtocolError(const QString &error);

signals:
    void deviceFound(const QBluetoothDeviceInfo &device);
    void discoveryFinished();
    void connectionEstablished();
    void connectionLost();
    void dataReceived(Command command, const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    static constexpr int CONNECTION_TIMEOUT = 5000;
    static constexpr int RESPONSE_TIMEOUT = 5000;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *bluetoothSocket;
    QTimer *connectionTimer;
    QTimer *responseTimer;

    Command currentCommand;

    SCP *scpHandler;

    void resetSocket();
};

#endif // BLUETOOTHHANDLER_H
