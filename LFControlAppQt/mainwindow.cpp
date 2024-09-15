#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), bluetoothHandler(new BluetoothHandler(this))
{
    ui->setupUi(this);

    connect(bluetoothHandler, &BluetoothHandler::deviceFound, this, &MainWindow::captureDeviceProperties);
    connect(bluetoothHandler, &BluetoothHandler::discoveryFinished, this, &MainWindow::searchingFinished);
    connect(bluetoothHandler, &BluetoothHandler::connectionEstablished, this, &MainWindow::connectionEstablished);
    connect(bluetoothHandler, &BluetoothHandler::connectionLost, this, &MainWindow::connectionLost);
    connect(bluetoothHandler, &BluetoothHandler::dataReceived, this, &MainWindow::handleDataReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    bluetoothHandler->disconnectFromDevice();
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
    foreach (const QBluetoothDeviceInfo &device, bluetoothHandler->discoveredDevices()) // Use discovered devices from BluetoothHandler
    {
        if (device.name() == selectedDeviceName)
        {
            selectedDeviceInfo = device;
            break;
        }
    }

    if (selectedDeviceInfo.isValid())
    {
        bluetoothHandler->connectToDevice(selectedDeviceInfo.address());
    }
    else
    {
        qDebug() << "Selected device not found.";
    }
}

void MainWindow::on_pushButtonSearch_clicked()
{
    bluetoothHandler->startDeviceDiscovery();
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
    qDebug() << "Connection established successfully.";
}

void MainWindow::connectionLost()
{
    qDebug() << "Connection lost.";
}

void MainWindow::handleDataReceived(Command command, const QByteArray &data)
{
    qDebug() << "Data received: " << data.toHex();
    // Handle data as per the command received
}

void MainWindow::on_pushButtonAutoConnect_clicked()
{
    // bluetoothHandler->sendCommand(Command::Start); // Example command
}

void MainWindow::on_pushButtonStart_clicked()
{
    // bluetoothHandler->sendCommand(Command::Start); // Example command
}

void MainWindow::on_pushButtonStop_clicked()
{
    // bluetoothHandler->sendCommand(Command::Stop); // Example command
}

void MainWindow::on_pushButtonReset_clicked()
{
    bluetoothHandler->sendCommand(Command::Reset); // Example command
}

void MainWindow::on_pushButtonCalibrate_clicked()
{
    bluetoothHandler->sendCommand(Command::Calibrate); // Example command
}

void MainWindow::on_radioButtonDebugMode_clicked(bool checked)
{
    // Handle debug mode toggle
}

void MainWindow::on_pushButtonReadNvm_clicked()
{
    // bluetoothHandler->sendCommand(Command::ReadNvm); // Example command to read NVM
}

void MainWindow::on_pushButtonWriteNvm_clicked()
{
    // bluetoothHandler->sendCommand(Command::WriteNvm); // Example command to write NVM
}
