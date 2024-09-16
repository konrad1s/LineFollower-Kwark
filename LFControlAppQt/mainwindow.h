#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QTimer>
#include "nvmlayout.h"
#include "bluetoothhandler.h"

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

private:
    Ui::MainWindow *ui;
    BluetoothHandler *bluetoothHandler;

    void updateNvmLayout(const QByteArray &data);
};
#endif // MAINWINDOW_H
