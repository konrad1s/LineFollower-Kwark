#include "qledindicator.h"
#include <QPainter>

QLedIndicator::QLedIndicator(QWidget *parent)
    : QWidget(parent), m_on(false), m_color(Qt::green)
{
}

bool QLedIndicator::isOn() const
{
    return m_on;
}

void QLedIndicator::setOn(bool on)
{
    if (m_on != on)
    {
        m_on = on;
        emit onChanged(m_on);
        update();
    }
}

QColor QLedIndicator::color() const
{
    return m_color;
}

void QLedIndicator::setColor(const QColor &color)
{
    if (m_color != color)
    {
        m_color = color;
        emit colorChanged(m_color);
        update();
    }
}

QSize QLedIndicator::sizeHint() const
{
    return QSize(100, 100);
}

QSize QLedIndicator::minimumSizeHint() const
{
    return QSize(50, 50);
}

void QLedIndicator::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    int side = qMin(width(), height());
    setFixedSize(side, side);
}

void QLedIndicator::paintEvent(QPaintEvent *event)
{
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_on)
    {
        painter.setBrush(m_color);
    }
    else
    {
        painter.setBrush(Qt::darkGray);
    }
    painter.drawEllipse(0, 0, width(), height());

    QPainter widgetPainter(this);
    widgetPainter.drawPixmap(0, 0, pixmap);
}
