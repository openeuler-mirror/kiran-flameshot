#include "toowidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <QComboBox>

Toowidget::Toowidget(QColor m_color,QWidget *parent) : QWidget(parent)
{
   setCursor(Qt::ArrowCursor);
   this->setMouseTracking(true);

   ConfigHandler config;
   m_colorList = config.getUserColors();
   m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
   m_colorAreaList = handleMask();
   m_selectColor = m_color;
   m_drawColor = m_color;

   update();
}

void Toowidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //QVector<QRect> rects = handleMask();
    painter.setPen(Qt::NoPen);

    painter.setBrush(m_drawColor);
    painter.drawRect(m_colorAreaList.at(0).x() - 30, m_colorAreaList.at(0).y() - 4, 20, 20);
    if(m_drawColor == Qt::white)
    {
        QPen pen = QPen();
        pen.setColor(Qt::black);
        pen.setWidthF(0.3);
        painter.setPen(pen);
        painter.drawRect(m_colorAreaList.at(0).x() - 30, m_colorAreaList.at(0).y() -4, 20, 20);
        painter.setPen(Qt::NoPen);
    }

    for (int i = 0; i < m_colorAreaList.size(); ++i) {
        painter.setBrush(QColor(m_colorList.at(i)));
        //painter.drawRoundRect(rects.at(i), 100, 100);
        painter.drawRect(m_colorAreaList.at(i));
        if(QColor(m_colorList.at(i)) == Qt::white)
        {
            QPen pen = QPen();
            pen.setColor(Qt::black);
            pen.setWidthF(0.3);
            painter.setPen(pen);
            painter.drawRect(m_colorAreaList.at(i));
            painter.setPen(Qt::NoPen);
        }
         //draw the highlight when we have to draw the selected color
        if (m_drawColor == QColor(m_colorList.at(i))) {
            if(m_drawColor == Qt::white)
            {
                QPen pen = QPen();
                pen.setColor(Qt::black);
                pen.setWidthF(0.5);
                painter.setPen(pen);
                painter.drawRect(m_colorAreaList.at(i).x() + 3, m_colorAreaList.at(i).y() + 3, 6, 6);
                painter.setPen(Qt::NoPen);
            }
            else{
                painter.setBrush(Qt::white);
                painter.drawRect(m_colorAreaList.at(i).x() + 3, m_colorAreaList.at(i).y() + 3, 6, 6);
            }
        }
        if(m_selectColor == QColor(m_colorList.at(i)) && m_selectColor != m_drawColor)
        {
            if(m_selectColor == Qt::white)
            {
                QPen pen = QPen();
                pen.setColor(Qt::black);
                pen.setWidthF(0.5);
                painter.setPen(pen);
                painter.drawRect(m_colorAreaList.at(i).x() + 3, m_colorAreaList.at(i).y() + 3, 6, 6);
                painter.setPen(Qt::NoPen);
            }
            else{
                painter.setBrush(Qt::white);
                painter.drawRect(m_colorAreaList.at(i).x() + 3, m_colorAreaList.at(i).y() + 3, 6, 6);
            }
        }
    }
}

void Toowidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
    {
        for (int i = 0; i < m_colorList.size(); ++i) {
                if (m_colorAreaList.at(i).contains(e->pos())) {
                    m_drawColor = m_colorList.at(i);
                    emit colorSelected(m_drawColor);
                    update();
                    break;
                }
            }
    }
}

void Toowidget::mouseMoveEvent(QMouseEvent *e) {
    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_selectColor = m_colorList.at(i);
            //emit colorSelected(m_drawColor);
            update();
            break;
        }
        m_selectColor = Qt::gray;
        update();
    }
}

QVector<QRect> Toowidget::handleMask() const {
    QVector<QRect> areas;
    for (int i = 0; i < m_colorList.size(); ++i) {
        QRect *rect = new QRect(102 + i * 22, 9, 12, 12);
        areas.append(*rect);
    }
    return areas;
}

void Toowidget::updateColor(QColor c)
{
    m_drawColor = c;
    update();
}
