#ifndef QLEDINDICATOR_H
#define QLEDINDICATOR_H

#include <QWidget>
#include <QColor>

class QLedIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool on READ isOn WRITE setOn NOTIFY onChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit QLedIndicator(QWidget *parent = nullptr);

    bool isOn() const;
    void setOn(bool on);

    QColor color() const;
    void setColor(const QColor &color);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void onChanged(bool on);
    void colorChanged(const QColor &color);

private:
    bool m_on;
    QColor m_color;
};

#endif // QLEDINDICATOR_H
