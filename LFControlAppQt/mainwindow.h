#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QTimer>
#include <QtCharts>
#include <QLineSeries>
#include <QValueAxis>
#include "nvmlayout.h"
#include "bluetoothhandler.h"
#include "plot.h"
#include "bootloader.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonConnect_clicked();
    void on_pushButtonSearch_clicked();
    void captureDeviceProperties(const QBluetoothDeviceInfo &device);
    void searchingFinished();
    void connectionEstablished();
    void connectionLost();
    void handleDataReceived(Command command, const QByteArray &data);
    void on_pushButtonAutoConnect_clicked();
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonReset_clicked();
    void on_pushButtonCalibrate_clicked();
    void on_radioButtonDebugMode_clicked(bool checked);
    void on_pushButtonReadNvm_clicked();
    void on_pushButtonWriteNvm_clicked();
    void on_pushButtonClearLogs_clicked();
    void on_pushButtonSaveLogs_clicked();
    void on_pushButtonBootEnter_clicked();
    void on_pushButtonBootReadVersion_clicked();
    void on_pushButtonBootSelectFile_clicked();
    void on_pushButtonBootErase_clicked();
    void on_pushButtonBootFlash_clicked();
    void on_pushButtonBootJumpApp_clicked();
    void on_pushButtonBootGetSession_clicked();

private:
    Ui::MainWindow *ui;
    BluetoothHandler *bluetoothHandler;
    Bootloader *bootloader;
    bool autoConnectInProgress;
    QString autoConnectDeviceName;

    Plot *plot1;
    Plot *plot2;
    size_t plotStartTime;

    void updateNvmLayout(const QByteArray &data);
    void updateDebugData(const QByteArray &data);
    void addToLogs(const QString &msg, bool isDebugMsg);
};
#endif // MAINWINDOW_H
