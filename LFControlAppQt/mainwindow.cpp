#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    bluetoothSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);

    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(captureDeviceProperties(QBluetoothDeviceInfo)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(searchingFinished));
    connect(bluetoothSocket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(bluetoothSocket, SIGNAL(disconnected()), this, SLOT(connectionLost()));
    connect(bluetoothSocket, SIGNAL(readyRead()), this, SLOT(readSocketData()));
    connect(connectionTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    if (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState)
    {
        bluetoothSocket->disconnectFromService();
        qDebug() << "Disconnected from the device.";
    }
}
void MainWindow::on_pushButtonConnect_clicked()
{
    QString selectedDeviceName = ui->comboBoxDevices->currentText();

    if (selectedDeviceName.isEmpty())
    {
        qDebug() << "No device selected.";
        return;
    }

    QBluetoothDeviceInfo selectedDeviceInfo;
    foreach (const QBluetoothDeviceInfo &device, discoveryAgent->discoveredDevices())
    {
        if (device.name() == selectedDeviceName)
        {
            selectedDeviceInfo = device;
            break;
        }
    }

    if (selectedDeviceInfo.isValid())
    {
        QBluetoothAddress address = selectedDeviceInfo.address();

        if (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState)
        {
            qDebug() << "Already connected.";
            return;
        }

        qDebug() << "Attempting to connect to device: " << selectedDeviceName;
        bluetoothSocket->connectToService(address, QBluetoothUuid::ServiceClassUuid::SerialPort);
        connectionTimer->start(CONNECTION_TIMEOUT);
    }
    else
    {
        qDebug() << "Selected device not found.";
    }
}

void MainWindow::on_pushButtonAutoConnect_clicked()
{
    QString predefinedDeviceAddress = "00:11:22:33:44:55";

    QBluetoothAddress address(predefinedDeviceAddress);

    if (bluetoothSocket->state() == QBluetoothSocket::SocketState::ConnectedState)
    {
        qDebug() << "Already connected.";
        return;
    }

    qDebug() << "Attempting to connect to predefined device: " << predefinedDeviceAddress;
    bluetoothSocket->connectToService(address, QBluetoothUuid::ServiceClassUuid::SerialPort);
    connectionTimer->start(CONNECTION_TIMEOUT);
}

void MainWindow::on_pushButtonSearch_clicked()
{
    discoveryAgent->start();
    ui->comboBoxDevices->clear();
    ui->pushButtonSearch->setEnabled(false);
    qDebug() << "Searching started";
}

void MainWindow::captureDeviceProperties(const QBluetoothDeviceInfo &device)
{
    ui->comboBoxDevices->addItem(device.name());
    qDebug() << "Device found: " << device.name();
}

void MainWindow::searchingFinished()
{
    ui->pushButtonSearch->setEnabled(true);
    qDebug() << "Searching finished";
}

void MainWindow::connectionEstablished()
{
    connectionTimer->stop();
    qDebug() << "Connection established successfully.";
}

void MainWindow::connectionLost()
{
    qDebug() << "Connection lost.";
}

void MainWindow::connectionTimeout()
{
    if (bluetoothSocket->state() != QBluetoothSocket::SocketState::ConnectedState)
    {
        qDebug() << "Connection attempt timed out.";
        bluetoothSocket->abort();
    }
}

void MainWindow::readSocketData()
{
    const auto data = bluetoothSocket->readAll();
    qDebug() << "Data received: " << data;
}
