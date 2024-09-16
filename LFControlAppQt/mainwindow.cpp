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
    foreach (const QBluetoothDeviceInfo &device, bluetoothHandler->discoveredDevices())
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
    switch (command)
    {
    case Command::ReadNvmData:
    {
        updateNvmLayout(data);
        break;
    }
    /* TODO: Currently only ReadNvmData is supported*/
    default:
        break;
    }
}

void MainWindow::on_pushButtonAutoConnect_clicked()
{
    // bluetoothHandler->sendCommand(Command::Start);
}

void MainWindow::on_pushButtonStart_clicked()
{
    QByteArray data;
    data.append(static_cast<char>(CommandSetMode::Start));

    bluetoothHandler->sendCommand(Command::SetMode, data);
}

void MainWindow::on_pushButtonStop_clicked()
{
    QByteArray data;
    data.append(static_cast<char>(CommandSetMode::Stop));

    bluetoothHandler->sendCommand(Command::SetMode, data);
}

void MainWindow::on_pushButtonReset_clicked()
{
    bluetoothHandler->sendCommand(Command::Reset, nullptr);
}

void MainWindow::on_pushButtonCalibrate_clicked()
{
    bluetoothHandler->sendCommand(Command::Calibrate, nullptr);
}

void MainWindow::on_radioButtonDebugMode_clicked(bool checked)
{
    // Handle debug mode toggle
}

void MainWindow::on_pushButtonReadNvm_clicked()
{
    bluetoothHandler->sendCommand(Command::ReadNvmData, nullptr);
}

void MainWindow::on_pushButtonWriteNvm_clicked()
{
    // bluetoothHandler->sendCommand(Command::WriteNvm);
}

void MainWindow::updateNvmLayout(const QByteArray &data)
{
    NVMLayout nvmLayout;

    qDebug() << "Data received: " << data.toHex();

    nvmLayout.parseFromArray(reinterpret_cast<const uint8_t *>(data.data()));

    QLineEdit *const sensorLineEdits[NVMLayout::SENSORS_NUMBER] = {
        ui->lineEditSensorValue1, ui->lineEditSensorValue2, ui->lineEditSensorValue3,
        ui->lineEditSensorValue4, ui->lineEditSensorValue5, ui->lineEditSensorValue6,
        ui->lineEditSensorValue7, ui->lineEditSensorValue8, ui->lineEditSensorValue9,
        ui->lineEditSensorValue10, ui->lineEditSensorValue11, ui->lineEditSensorValue12};

    for (int i = 0; i < NVMLayout::SENSORS_NUMBER; ++i)
    {
        sensorLineEdits[i]->setText(QString::number(nvmLayout.sensorWeights[i]));
    }

    ui->lineEditErrorThreshold->setText(QString::number(nvmLayout.errorThreshold));
    ui->lineEditfallbackPositive->setText(QString::number(nvmLayout.fallbackErrorPositive));
    ui->lineEditfallbackNegative->setText(QString::number(nvmLayout.fallbackErrorNegative));

    struct PidSettings
    {
        const PIDSettings &pid;
        QLineEdit *lineEditKp;
        QLineEdit *lineEditKi;
        QLineEdit *lineEditKd;
        QLineEdit *lineEditIntMax;
        QLineEdit *lineEditIntMin;
        QLineEdit *lineEditOutputMax;
        QLineEdit *lineEditOutputMin;
    };

    PidSettings pidSettings[] = {
        {nvmLayout.pidStgSensor, ui->lineEditPid1Kp, ui->lineEditPid1Ki, ui->lineEditPid1Kd,
         ui->lineEditPid1IntMax, ui->lineEditPid1IntMin, ui->lineEditPid1OutputMax, ui->lineEditPid1OutputMin},
        {nvmLayout.pidStgMotorLeft, ui->lineEditPid2Kp, ui->lineEditPid2Ki, ui->lineEditPid2Kd,
         ui->lineEditPid2IntMax, ui->lineEditPid2IntMin, ui->lineEditPid2OutputMax, ui->lineEditPid2OutputMin},
        {nvmLayout.pidStgMotorRight, ui->lineEditPid3Kp, ui->lineEditPid3Ki, ui->lineEditPid3Kd,
         ui->lineEditPid3IntMax, ui->lineEditPid3IntMin, ui->lineEditPid3OutputMax, ui->lineEditPid3OutputMin}};

    for (const PidSettings &settings : pidSettings)
    {
        settings.lineEditKp->setText(QString::number(settings.pid.kp));
        settings.lineEditKi->setText(QString::number(settings.pid.ki));
        settings.lineEditKd->setText(QString::number(settings.pid.kd));
        settings.lineEditIntMax->setText(QString::number(settings.pid.integralMax));
        settings.lineEditIntMin->setText(QString::number(settings.pid.integralMin));
        settings.lineEditOutputMax->setText(QString::number(settings.pid.outputMax));
        settings.lineEditOutputMin->setText(QString::number(settings.pid.outputMin));
    }
}