#include "plot.h"

Plot::Plot(QWidget *parent, const QString &title, const QString &xTitle, const QString &yTitle)
    : QWidget(parent), xMin(0), xMax(0)
{
    chart = new QChart();
    chart->setTitle(title);

    axisX = new QValueAxis();
    axisX->setTitleText(xTitle);
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis();
    axisY->setTitleText(yTitle);
    chart->addAxis(axisY, Qt::AlignLeft);

    defaultSeries = new QLineSeries();
    defaultSeries->setUseOpenGL(true);
    chart->addSeries(defaultSeries);
    defaultSeries->attachAxis(axisX);
    defaultSeries->attachAxis(axisY);
    seriesList.append(defaultSeries);
    dataBuffers.append(QVector<QPointF>());

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Plot::updateChart);
    updateTimer->start(50);
}

void Plot::clear()
{
    for (QLineSeries *series : seriesList)
    {
        series->clear();
    }
}

void Plot::setSeriesName(const QString &seriesName)
{
    setSeriesName(0, seriesName);
}

void Plot::setSeriesName(int seriesIndex, const QString &seriesName)
{
    if (seriesIndex >= 0 && seriesIndex < seriesList.size())
    {
        seriesList[seriesIndex]->setName(seriesName);
    }
    else
    {
        qWarning("Series index out of bounds");
    }
}

void Plot::setSeriesPen(int seriesIndex, const QPen &pen)
{
    if (seriesIndex >= 0 && seriesIndex < seriesList.size())
    {
        seriesList[seriesIndex]->setPen(pen);
    }
    else
    {
        qWarning("Series index out of bounds");
    }
}

void Plot::addDataPoint(qreal x, qreal y)
{
    addDataPoint(0, x, y);
}

void Plot::addDataPoint(int seriesIndex, qreal x, qreal y)
{
    if (seriesIndex >= 0 && seriesIndex < seriesList.size())
    {
        dataBuffers[seriesIndex].append(QPointF(x, y));

        if (seriesList[seriesIndex]->points().isEmpty())
        {
            xMin = x;
            xMax = x;
        }
        else
        {
            xMax = x;
        }
    }
    else
    {
        qWarning("Series index out of bounds");
    }
}

int Plot::addSeries(const QString &seriesName)
{
    QLineSeries *newSeries = new QLineSeries();

    if (!seriesName.isEmpty())
    {
        newSeries->setName(seriesName);
    }

    newSeries->setUseOpenGL(true);
    chart->addSeries(newSeries);
    newSeries->attachAxis(axisX);
    newSeries->attachAxis(axisY);
    seriesList.append(newSeries);
    dataBuffers.append(QVector<QPointF>());

    return seriesList.size() - 1;
}

void Plot::setAxisRange(qreal xMin, qreal xMax, qreal yMin, qreal yMax)
{
    axisX->setRange(xMin, xMax);
    axisY->setRange(yMin, yMax);
}

void Plot::updateChart()
{
    for (int seriesId = 0; seriesId < seriesList.size(); ++seriesId)
    {
        if (!dataBuffers[seriesId].isEmpty())
        {
            auto *const series = seriesList[seriesId];

            series->append(dataBuffers[seriesId]);

            if (series->count() > maxDataPoints)
            {
                int removeCount = series->count() - maxDataPoints;
                series->removePoints(0, removeCount);
                xMin = series->at(0).x();
            }

            dataBuffers[seriesId].clear();
        }
    }

    axisX->setRange(xMin, xMax);
}
