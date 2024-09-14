#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
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
    void on_pushButtonAutoConnect_clicked();
    void connectionEstablished();
    void connectionLost();
    void connectionTimeout();
    void readSocketData();

private:
    constexpr static int CONNECTION_TIMEOUT = 10000;

    Ui::MainWindow *ui;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *bluetoothSocket;
    QTimer *connectionTimer;
};
#endif // MAINWINDOW_H
