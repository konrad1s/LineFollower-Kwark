#include "plot.h"

Plot::Plot(QWidget *parent, const QString &title, const QString &xTitle, const QString &yTitle)
    : QWidget(parent), autoRangeEnabled(true)
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

    xMin = std::numeric_limits<qreal>::max();
    xMax = std::numeric_limits<qreal>::lowest();
    yMin = std::numeric_limits<qreal>::max();
    yMax = std::numeric_limits<qreal>::lowest();
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

        if (autoRangeEnabled)
        {
            if (x < xMin) xMin = x;
            if (x > xMax) xMax = x;
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
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
    this->xMin = xMin;
    this->xMax = xMax;
    this->yMin = yMin;
    this->yMax = yMax;

    axisX->setRange(xMin, xMax);
    axisY->setRange(yMin, yMax);

    autoRangeEnabled = false;
}

void Plot::enableAutoRange(bool enable)
{
    autoRangeEnabled = enable;

    if (autoRangeEnabled)
    {
        xMin = std::numeric_limits<qreal>::max();
        xMax = std::numeric_limits<qreal>::lowest();
        yMin = std::numeric_limits<qreal>::max();
        yMax = std::numeric_limits<qreal>::lowest();

        for (QLineSeries *series : seriesList)
        {
            for (const QPointF &point : series->pointsVector())
            {
                if (point.x() < xMin) xMin = point.x();
                if (point.x() > xMax) xMax = point.x();
                if (point.y() < yMin) yMin = point.y();
                if (point.y() > yMax) yMax = point.y();
            }
        }

        axisX->setRange(xMin, xMax);
        axisY->setRange(yMin, yMax);
    }
}

void Plot::updateChart()
{
    bool rangeChanged = false;

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

                if (autoRangeEnabled)
                {
                    QPointF firstPoint = series->at(0);
                    xMin = firstPoint.x();
                }
            }

            dataBuffers[seriesId].clear();
            rangeChanged = true;
        }
    }

    if (autoRangeEnabled && rangeChanged)
    {
        qDebug() << xMin << xMax;
        axisX->setRange(xMin, xMax);
        axisY->setRange(yMin, yMax);
    }
}
