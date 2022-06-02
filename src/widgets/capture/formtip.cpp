#include "formtip.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

FormTip::FormTip(QWidget *parent) : QWidget(parent)
{
    setCursor(Qt::ArrowCursor);
    this->setMouseTracking(true);
    this->resize(535,20);
}

void FormTip::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    if(m_isReverse)
    {
        path.moveTo(QPoint(point.x(), point.y() + 6));
        path.lineTo(QPoint(point.x() -6, point.y()));
        path.lineTo(QPoint(point.x() + 6, point.y()));
        path.lineTo(QPoint(point.x() , point.y() + 6));
    }
    else
    {
        path.moveTo(QPoint(point.x(), point.y() + 3));
        path.lineTo(QPoint(point.x() -6, point.y() + 9));
        path.lineTo(QPoint(point.x() + 6, point.y() + 9));
        path.lineTo(QPoint(point.x() , point.y() + 3));
    }
    painter.fillPath(path,QBrush(Qt::white));
}

void FormTip::show(const QPoint p,bool isReverse)
{
    point = p;
    m_isReverse = isReverse;
    update();

    QWidget::show();
}

void FormTip::hide()
{
    QWidget::hide();
}
