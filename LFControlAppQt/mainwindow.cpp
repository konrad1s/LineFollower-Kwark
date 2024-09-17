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
    connect(bluetoothHandler, &BluetoothHandler::errorOccurred, [this](const QString &error) { qDebug() << error; });
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
    ui->indicatorConnStatus->setOn(true);
    qDebug() << "Connection established successfully.";
}

void MainWindow::connectionLost()
{
    ui->indicatorConnStatus->setOn(false);
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

    QLineEdit *const sensorWeights[NVMLayout::SENSORS_NUMBER] = {
        ui->lineEditSensorWeight1, ui->lineEditSensorWeight2, ui->lineEditSensorWeight3,
        ui->lineEditSensorWeight4, ui->lineEditSensorWeight5, ui->lineEditSensorWeight6,
        ui->lineEditSensorWeight7, ui->lineEditSensorWeight8, ui->lineEditSensorWeight9,
        ui->lineEditSensorWeight10, ui->lineEditSensorWeight11, ui->lineEditSensorWeight12};
    QLineEdit *const sensorThresholds[NVMLayout::SENSORS_NUMBER] = {
        ui->lineEditSensorCalib1, ui->lineEditSensorCalib2, ui->lineEditSensorCalib3,
        ui->lineEditSensorCalib4, ui->lineEditSensorCalib5, ui->lineEditSensorCalib6,
        ui->lineEditSensorCalib7, ui->lineEditSensorCalib8, ui->lineEditSensorCalib9,
        ui->lineEditSensorCalib10, ui->lineEditSensorCalib11, ui->lineEditSensorCalib12};

    for (int i = 0; i < NVMLayout::SENSORS_NUMBER; ++i)
    {
        sensorWeights[i]->setText(QString::number(nvmLayout.sensors.weights[i]));
        sensorThresholds[i]->setText(QString::number(nvmLayout.sensors.thresholds[i]));
    }

    ui->lineEditErrorThreshold->setText(QString::number(nvmLayout.sensors.errorThreshold));
    ui->lineEditfallbackPositive->setText(QString::number(nvmLayout.sensors.fallbackErrorPositive));
    ui->lineEditfallbackNegative->setText(QString::number(nvmLayout.sensors.fallbackErrorNegative));

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