#include "bluetoothhandler.h"

BluetoothHandler::BluetoothHandler(QObject *parent)
    : QObject{parent}, discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this)), bluetoothSocket(new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this)),
      connectionTimer(new QTimer(this)), responseTimer(new QTimer(this))
{
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(handleDeviceDiscovered(QBluetoothDeviceInfo)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(handleDiscoveryFinished()));
    connect(bluetoothSocket, SIGNAL(disconnected()), this, SLOT(handleConnectionLost()));
    connect(bluetoothSocket, SIGNAL(connected()), this, SLOT(handleConnectionEstablished()));
    connect(bluetoothSocket, SIGNAL(readyRead()), this, SLOT(handleSocketReadyRead()));
    connect(connectionTimer, SIGNAL(timeout()), this, SLOT(handleConnectionTimeout));
    connect(responseTimer, SIGNAL(timeout()), this, SLOT(handleResponseTimeout));

    connectionTimer->setSingleShot(true);
    responseTimer->setSingleShot(true);
}

void BluetoothHandler::startDeviceDiscovery()
{
    discoveryAgent->start();
}

void BluetoothHandler::connectToDevice(const QBluetoothAddress &address)
{
    if (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState)
    {
        qDebug() << "Already connected.";
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
        // TODO: emmit error
        return;
    }

    QByteArray commandData;
    QDataStream stream(&commandData, QIODevice::WriteOnly);
    stream << command;

    if (!data.isEmpty())
    {
        commandData.append(data);
    }

    bluetoothSocket->write(commandData);
    currentCommand = command;
    responseTimer->start(RESPONSE_TIMEOUT);
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
    dataBuffer.append(bluetoothSocket->readAll());

    const qsizetype dataSize = dataBuffer.size();
    const qsizetype expectedResponseSize = commandResponseSize.at(currentCommand);
    const int expectedResponseCommand = static_cast<int>(currentCommand) + 1;

    if ((dataSize == expectedResponseSize) && (dataBuffer.at(0) == expectedResponseCommand))
    {
        responseTimer->stop();
        processReceivedData();
        dataBuffer.clear();
        currentCommand = Command::InvalidCommand;
    }
    else if ((dataSize > expectedResponseSize) || (dataSize > 0U && dataBuffer.at(0) != expectedResponseCommand))
    {
        responseTimer->stop();
        dataBuffer.clear();
        currentCommand = Command::InvalidCommand;
    }
}

void BluetoothHandler::handleConnectionTimeout()
{
    if (!isConnected())
    {
        // TODO: emit connection timeout
        disconnectFromDevice();
    }
}

void BluetoothHandler::handleResponseTimeout()
{
    //TODO: emit error
    currentCommand = Command::InvalidCommand;
    dataBuffer.clear();
}

void BluetoothHandler::processReceivedData()
{
    emit dataReceived(currentCommand, dataBuffer);
}
