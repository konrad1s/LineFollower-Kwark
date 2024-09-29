#include "bluetoothhandler.h"
#include <QBluetoothUuid>
#include <QDataStream>

BluetoothHandler::BluetoothHandler(QObject *parent)
    : QObject{parent},
    discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this)),
    bluetoothSocket(new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this)),
    connectionTimer(new QTimer(this)),
    responseTimer(new QTimer(this)),
    currentCommand(Command::InvalidCommand),
    scpHandler(new SCP(this))
{
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothHandler::handleDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BluetoothHandler::handleDiscoveryFinished);

    connect(bluetoothSocket, &QBluetoothSocket::connected, this, &BluetoothHandler::handleConnectionEstablished);
    connect(bluetoothSocket, &QBluetoothSocket::disconnected, this, &BluetoothHandler::handleConnectionLost);
    connect(bluetoothSocket, &QBluetoothSocket::readyRead, this, &BluetoothHandler::handleSocketReadyRead);

    connect(connectionTimer, &QTimer::timeout, this, &BluetoothHandler::handleConnectionTimeout);
    connect(responseTimer, &QTimer::timeout, this, &BluetoothHandler::handleResponseTimeout);

    connectionTimer->setSingleShot(true);
    responseTimer->setSingleShot(true);

    connect(scpHandler, &SCP::packetReadyToSend, this, &BluetoothHandler::handlePacketReadyToSend);
    connect(scpHandler, &SCP::commandReceived, this, &BluetoothHandler::handleCommandReceived);
    connect(scpHandler, &SCP::errorOccurred, this, &BluetoothHandler::handleProtocolError);
}

BluetoothHandler::~BluetoothHandler()
{
    disconnectFromDevice();
}

void BluetoothHandler::startDeviceDiscovery()
{
    discoveryAgent->start();
}

void BluetoothHandler::stopDeviceDiscovery()
{
    discoveryAgent->stop();
}

void BluetoothHandler::connectToDevice(const QBluetoothAddress &address)
{
    if (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState)
    {
        emit errorOccurred("Already connected to a device.");
        return;
    }
    bluetoothSocket->connectToService(address, QBluetoothUuid::ServiceClassUuid::SerialPort);
    connectionTimer->start(CONNECTION_TIMEOUT);
}

void BluetoothHandler::disconnectFromDevice()
{
    if (isConnected())
    {
        bluetoothSocket->disconnectFromService();
    }
}

bool BluetoothHandler::isConnected() const
{
    return (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState);
}

void BluetoothHandler::sendCommand(Command command, const QByteArray &data)
{
    if (!bluetoothSocket->isOpen())
    {
        emit errorOccurred("Socket is not open.");
        return;
    }

    if (currentCommand != Command::InvalidCommand)
    {
        emit errorOccurred("Previous command is still pending.");
        return;
    }

    currentCommand = command;
    responseTimer->start(RESPONSE_TIMEOUT);
    scpHandler->sendCommand(command, data);
}

QList<QBluetoothDeviceInfo> BluetoothHandler::discoveredDevices() const
{
    return discoveryAgent->discoveredDevices();
}

void BluetoothHandler::handleDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    emit deviceFound(device);
}

void BluetoothHandler::handleDiscoveryFinished()
{
    emit discoveryFinished();
}

void BluetoothHandler::handleConnectionEstablished()
{
    connectionTimer->stop();
    emit connectionEstablished();
}

void BluetoothHandler::handleConnectionLost()
{
    emit connectionLost();
}

void BluetoothHandler::handleSocketReadyRead()
{
    QByteArray data = bluetoothSocket->readAll();
    scpHandler->receiveData(data);
}

void BluetoothHandler::handleConnectionTimeout()
{
    if (!isConnected())
    {
        emit errorOccurred("Connection timeout occurred.");
        disconnectFromDevice();
    }
}

void BluetoothHandler::handleResponseTimeout()
{
    emit errorOccurred("Response timeout occurred.");
    currentCommand = Command::InvalidCommand;
}

void BluetoothHandler::handlePacketReadyToSend(const QByteArray &packet)
{
    bluetoothSocket->write(packet);
}

void BluetoothHandler::handleCommandReceived(Command command, const QByteArray &data)
{
    if (command == currentCommand)
    {
        responseTimer->stop();
        currentCommand = Command::InvalidCommand;
    }
    emit dataReceived(command, data);
}

void BluetoothHandler::handleProtocolError(const QString &error)
{
    emit errorOccurred(error);
}
