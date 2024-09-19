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
    else
    {
        emit errorOccurred("No device to disconnect.");
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

    if (dataSize < ACK_RESPONSE_SIZE) {
        return;
    }

    int receivedCommand = static_cast<uint8_t>(dataBuffer.at(0)) | (static_cast<uint8_t>(dataBuffer.at(1)) << 8);

    qsizetype expectedResponseSize = 0;
    if (commandResponseSize.contains(currentCommand))
    {
        expectedResponseSize = commandResponseSize.at(currentCommand);
    }
    else
    {
        emit errorOccurred("Unexpected command received, no matching entry in response size map.");
        return;
    }

    int expectedResponseCommand = static_cast<int>(currentCommand) + 1;

    if ((dataSize == expectedResponseSize) && (receivedCommand == expectedResponseCommand))
    {
        responseTimer->stop();
        processReceivedData();
        dataBuffer.clear();
        currentCommand = Command::InvalidCommand;
    }
    else if ((dataSize > expectedResponseSize) || (dataSize > 0U && receivedCommand != expectedResponseCommand))
    {
        responseTimer->stop();
        dataBuffer.clear();
        currentCommand = Command::InvalidCommand;
        emit errorOccurred("Received incorrect or mismatched data, expected response size: " +
                           QString::number(expectedResponseSize) + ", received data size: " + QString::number(dataSize));
    }
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
    dataBuffer.clear();
}

void BluetoothHandler::processReceivedData()
{
    emit dataReceived(currentCommand, dataBuffer.mid(ACK_RESPONSE_SIZE));
}
