#ifndef PLOT_H
#define PLOT_H

#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QChartView>
#include <QVector>
#include <QVBoxLayout>
#include <QTimer>

class Plot : public QWidget
{
public:
    explicit Plot(QWidget *parent = nullptr, const QString &title = "", const QString &xTitle = "", const QString &yTitle = "");
    ~Plot() = default;

    void clear();
    void setSeriesName(const QString &seriesName);
    void setSeriesName(int seriesIndex, const QString &seriesName);
    void addDataPoint(qreal x, qreal y);
    void addDataPoint(int seriesIndex, qreal x, qreal y);
    int addSeries(const QString &seriesName = "");
    void setAxisRange(qreal xMin, qreal xMax, qreal yMin, qreal yMax);

private slots:
    void updateChart();

private:
    const int maxDataPoints = 1000;

    QChart *chart;
    QChartView *chartView;
    QVector<QLineSeries *> seriesList;
    QLineSeries *defaultSeries;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QTimer *updateTimer;
    QVector<QVector<QPointF>> dataBuffers;
    qreal xMin, xMax;
};

#endif // PLOT_H
