#include "thicknesswidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "baseutils.h"

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

ThicknessWidget::ThicknessWidget(int thickness,QWidget *parent) : QWidget(parent)
{
   setCursor(Qt::ArrowCursor);
   this->setMouseTracking(true);

   ConfigHandler config;
   m_thinknessList = {SMALL_SIZE,MEDIUM_SIZE,LARGE_SIZE};
   //m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
   m_thinknessAreaList = handleMask();
   m_selectThinkness = thickness;
   m_drawThinkness = thickness;

   update();
}

void ThicknessWidget::paintEvent(QPaintEvent *) {
    if(m_ishide)
        return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //QVector<QRect> rects = handleMask();
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < m_thinknessAreaList.size(); ++i) {

        if (m_drawThinkness == int(m_thinknessList.at(i))) {
                painter.setBrush(QColor(56,168,255));
                painter.drawEllipse(m_thinknessAreaList.at(i));
        }
        else
        {
            painter.setBrush(Qt::gray);
            //painter.drawRoundRect(rects.at(i), 100, 100);
            painter.drawEllipse(m_thinknessAreaList.at(i));
             //draw the highlight when we have to draw the selected color
        }

        if(m_selectThinkness == int(m_thinknessList.at(i)) && m_selectThinkness != m_drawThinkness)
        {
                painter.setBrush(Qt::blue);
                painter.drawEllipse(m_thinknessAreaList.at(i));
        }
    }
}

void ThicknessWidget::mousePressEvent(QMouseEvent *e) {
    if(m_ishide)
        return;
    if (e->button() == Qt::LeftButton)
    {
        for (int i = 0; i < m_thinknessList.size(); ++i) {
                if (m_thinknessAreaList.at(i).contains(e->pos())) {
                    m_drawThinkness = m_thinknessList.at(i);
                    emit thicknessSelected(m_drawThinkness);
                    update();
                    break;
                }
            }
    }
}

void ThicknessWidget::mouseMoveEvent(QMouseEvent *e) {
    if(m_ishide)
        return;
    for (int i = 0; i < m_thinknessList.size(); ++i) {
        if (m_thinknessAreaList.at(i).contains(e->pos())) {
            m_selectThinkness = m_thinknessList.at(i);
            //emit colorSelected(m_drawColor);
            update();
            break;
        }
        m_selectThinkness = 0;
        update();
    }
}

QVector<QRect> ThicknessWidget::handleMask() const {
    QVector<QRect> areas;
    int spacing = 0;
    for (int i = 0; i < m_thinknessList.size(); ++i) {
        spacing += i*4;
        QRect *rect = new QRect(8 + i * 10 + spacing , 13 - i * 2  , (i+1)*4, (i+1)*4);
        areas.append(*rect);
    }
    return areas;
}

void ThicknessWidget::updateThickness(int size)
{
    m_drawThinkness = size;
    update();
}

void ThicknessWidget::hide()
{
    m_ishide = true;
    update();
}

void ThicknessWidget::show()
{
    m_ishide = false;
    update();
}
