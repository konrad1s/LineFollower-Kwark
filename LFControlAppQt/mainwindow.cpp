#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "debugdata.h"
#include <QDateTime.h>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), bluetoothHandler(new BluetoothHandler(this)),
      autoConnectInProgress(false)
{
    ui->setupUi(this);

    plot1 = new Plot(this, "Robot speed", "Time", "Speed");
    plot1->setSeriesName("Motor Left");
    plot1->addSeries("Motor Right");
    QVBoxLayout *tab1Layout = new QVBoxLayout(ui->tabChart1);
    tab1Layout->addWidget(plot1);
    ui->tabChart1->setLayout(tab1Layout);

    plot2 = new Plot(this, "Sensor error", "Time", "Error");
    QVBoxLayout *tab2Layout = new QVBoxLayout(ui->tabChart2);
    tab2Layout->addWidget(plot2);
    ui->tabChart2->setLayout(tab2Layout);

    plotStartTime = 0;

    connect(bluetoothHandler, &BluetoothHandler::deviceFound, this, &MainWindow::captureDeviceProperties);
    connect(bluetoothHandler, &BluetoothHandler::discoveryFinished, this, &MainWindow::searchingFinished);
    connect(bluetoothHandler, &BluetoothHandler::connectionEstablished, this, &MainWindow::connectionEstablished);
    connect(bluetoothHandler, &BluetoothHandler::connectionLost, this, &MainWindow::connectionLost);
    connect(bluetoothHandler, &BluetoothHandler::dataReceived, this, &MainWindow::handleDataReceived);
    connect(bluetoothHandler, &BluetoothHandler::errorOccurred, [this](const QString &error)
            { addToLogs(error, false); });
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
        addToLogs("No device selected.", false);
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
        addToLogs("Device found, connect attempting", true);
        bluetoothHandler->connectToDevice(selectedDeviceInfo.address());
    }
    else
    {
        addToLogs("Selected device not found.", false);
    }
}

void MainWindow::on_pushButtonSearch_clicked()
{
    bluetoothHandler->startDeviceDiscovery();
    ui->comboBoxDevices->clear();
    ui->pushButtonSearch->setEnabled(false);
    addToLogs("Bluetooth searching started", true);
}

void MainWindow::captureDeviceProperties(const QBluetoothDeviceInfo &device)
{
    ui->comboBoxDevices->addItem(device.name());
    addToLogs("Device found: " + device.name(), true);

    if (autoConnectInProgress)
    {
        if (device.name() == autoConnectDeviceName)
        {
            addToLogs("AutoConnect: Target device found: " + device.name(), true);

            bluetoothHandler->connectToDevice(device.address());

            autoConnectInProgress = false;
            ui->pushButtonAutoConnect->setEnabled(true);
        }
    }
}

void MainWindow::searchingFinished()
{
    ui->pushButtonSearch->setEnabled(true);
    addToLogs("Searching finished", true);
}

void MainWindow::connectionEstablished()
{
    ui->indicatorConnStatus->setOn(true);
    bluetoothHandler->stopDeviceDiscovery();
    addToLogs("Connection established successfully.", false);
}

void MainWindow::connectionLost()
{
    ui->indicatorConnStatus->setOn(false);
    addToLogs("Connection lost.", false);
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
    case Command::DebugData:
    {
        updateDebugData(data);
        break;
    }
    default:
        break;
    }
}

void MainWindow::on_pushButtonAutoConnect_clicked()
{
    addToLogs("AutoConnect started", true);

    autoConnectDeviceName = "HC-05";
    autoConnectInProgress = true;
    bluetoothHandler->startDeviceDiscovery();
    ui->pushButtonAutoConnect->setEnabled(false);
}

void MainWindow::on_pushButtonStart_clicked()
{
    QByteArray data;
    data.append(static_cast<char>(CommandSetMode::Start));
    bluetoothHandler->sendCommand(Command::SetMode, data);
    addToLogs("Start command sent", true);
}

void MainWindow::on_pushButtonStop_clicked()
{
    QByteArray data;
    data.append(static_cast<char>(CommandSetMode::Stop));
    bluetoothHandler->sendCommand(Command::SetMode, data);
    addToLogs("Stop command sent", true);
}

void MainWindow::on_pushButtonReset_clicked()
{
    bluetoothHandler->sendCommand(Command::Reset, nullptr);
    addToLogs("Reset command sent", true);
}

void MainWindow::on_pushButtonCalibrate_clicked()
{
    bluetoothHandler->sendCommand(Command::Calibrate, nullptr);
    addToLogs("Calibrate command sent", true);
}

void MainWindow::on_radioButtonDebugMode_clicked(bool checked)
{
    QByteArray data;

    if (checked)
    {
        plotStartTime = QDateTime::currentMSecsSinceEpoch();
        plot1->clear();
        plot2->clear();
        data.append(static_cast<char>(true));
        addToLogs("Debug mode enabled", true);
    }
    else
    {
        data.append(static_cast<char>(false));
        addToLogs("Debug mode disabled", true);
    }

    bluetoothHandler->sendCommand(Command::SetDebugMode, data);
}

void MainWindow::on_pushButtonReadNvm_clicked()
{
    bluetoothHandler->sendCommand(Command::ReadNvmData, nullptr);
    addToLogs("Read NvM command sent", true);
}

void MainWindow::on_pushButtonWriteNvm_clicked()
{
    NVMLayout nvmLayout;

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
        nvmLayout.sensors.weights[i] = sensorWeights[i]->text().toInt();
        nvmLayout.sensors.thresholds[i] = sensorThresholds[i]->text().toInt();
    }

    nvmLayout.sensors.errorThreshold = ui->lineEditErrorThreshold->text().toFloat();
    nvmLayout.sensors.fallbackErrorPositive = ui->lineEditfallbackPositive->text().toFloat();
    nvmLayout.sensors.fallbackErrorNegative = ui->lineEditfallbackNegative->text().toFloat();

    struct PidSettingsUI
    {
        PIDSettings &pid;
        QLineEdit *lineEditKp;
        QLineEdit *lineEditKi;
        QLineEdit *lineEditKd;
        QLineEdit *lineEditIntMax;
        QLineEdit *lineEditIntMin;
        QLineEdit *lineEditOutputMax;
        QLineEdit *lineEditOutputMin;
    };

    PidSettingsUI pidSettingsUI[] = {
        {nvmLayout.pidStgSensor, ui->lineEditPid1Kp, ui->lineEditPid1Ki, ui->lineEditPid1Kd,
         ui->lineEditPid1IntMax, ui->lineEditPid1IntMin, ui->lineEditPid1OutputMax, ui->lineEditPid1OutputMin},
        {nvmLayout.pidStgMotorLeft, ui->lineEditPid2Kp, ui->lineEditPid2Ki, ui->lineEditPid2Kd,
         ui->lineEditPid2IntMax, ui->lineEditPid2IntMin, ui->lineEditPid2OutputMax, ui->lineEditPid2OutputMin},
        {nvmLayout.pidStgMotorRight, ui->lineEditPid3Kp, ui->lineEditPid3Ki, ui->lineEditPid3Kd,
         ui->lineEditPid3IntMax, ui->lineEditPid3IntMin, ui->lineEditPid3OutputMax, ui->lineEditPid3OutputMin}};

    for (const PidSettingsUI &settings : pidSettingsUI)
    {
        settings.pid.kp = settings.lineEditKp->text().toFloat();
        settings.pid.ki = settings.lineEditKi->text().toFloat();
        settings.pid.kd = settings.lineEditKd->text().toFloat();
        settings.pid.integralMax = settings.lineEditIntMax->text().toFloat();
        settings.pid.integralMin = settings.lineEditIntMin->text().toFloat();
        settings.pid.outputMax = settings.lineEditOutputMax->text().toFloat();
        settings.pid.outputMin = settings.lineEditOutputMin->text().toFloat();
    }

    QByteArray data(nvmLayout.size(), Qt::Uninitialized);
    nvmLayout.serializeToArray(reinterpret_cast<uint8_t *>(data.data()));

    bluetoothHandler->sendCommand(Command::WriteNvmData, data);

    addToLogs("Write NvM command sent \n" + nvmLayout.toString(), true);
}

void MainWindow::updateNvmLayout(const QByteArray &data)
{
    NVMLayout nvmLayout;

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

    addToLogs("NvM layout updated: \n" + nvmLayout.toString(), true);
}

void MainWindow::updateDebugData(const QByteArray &data)
{
    DebugData debugData;

    debugData.parseFromArray(reinterpret_cast<const uint8_t *>(data.data()));

    QLineEdit *const sensorValues[DebugData::SENSORS_NUMBER] = {
        ui->lineEditSensorValue1, ui->lineEditSensorValue2, ui->lineEditSensorValue3, ui->lineEditSensorValue4, 
        ui->lineEditSensorValue5, ui->lineEditSensorValue6, ui->lineEditSensorValue7, ui->lineEditSensorValue8, 
        ui->lineEditSensorValue9, ui->lineEditSensorValue10, ui->lineEditSensorValue11, ui->lineEditSensorValue12};

    QLedIndicator *const sensorLeds[DebugData::SENSORS_NUMBER] = {
        ui->sensorLed1, ui->sensorLed2, ui->sensorLed3, ui->sensorLed4,
        ui->sensorLed5, ui->sensorLed6, ui->sensorLed7, ui->sensorLed8,
        ui->sensorLed9, ui->sensorLed10, ui->sensorLed11, ui->sensorLed12};

    for (int i = 0; i < DebugData::SENSORS_NUMBER; ++i)
    {
        sensorValues[i]->setText(QString::number(debugData.sensorValues[i]));
        sensorLeds[i]->setOn(debugData.sensorValues[i] > 3000);
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch() - plotStartTime;
    plot2->addDataPoint(currentTime, debugData.sensorError);

    addToLogs("Debug data updated: \n" + debugData.toString(), true);
}

void MainWindow::addToLogs(const QString &msg, bool isDebugMsg)
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    const QString msgLog = currentTime + "   " + msg;

    if (isDebugMsg)
    {
        if (ui->radioButtonDebugLogs->isChecked())
        {
            ui->textEditLogs->append(msgLog);
        }
    }
    else
    {
        ui->textEditLogs->append(msgLog);
    }
}

void MainWindow::on_pushButtonClearLogs_clicked()
{
    ui->textEditLogs->clear();
}

void MainWindow::on_pushButtonSaveLogs_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Logs"), "", tr("Text Files (*.txt);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream out(&file);
    out << ui->textEditLogs->toPlainText();

    file.close();

    QMessageBox::information(this, tr("Logs Saved"), tr("Logs have been saved to %1").arg(fileName));
}

void MainWindow::on_pushButtonBootEnter_clicked()
{

}


void MainWindow::on_pushButtonBootReadVersion_clicked()
{

}


void MainWindow::on_pushButtonBootSelectFile_clicked()
{

}


void MainWindow::on_pushButtonBootLoadKey_clicked()
{

}


void MainWindow::on_pushButtonBootErase_clicked()
{

}


void MainWindow::on_pushButtonBootFlash_clicked()
{

}


void MainWindow::on_pushButtonBootValidate_clicked()
{

}


void MainWindow::on_pushButtonBootJumpApp_clicked()
{

}

