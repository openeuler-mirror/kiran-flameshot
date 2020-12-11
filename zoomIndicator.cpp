/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "zoomIndicator.h"
#include "baseutils.h"
#include "src/tools/capturecontext.h"
//#include "tempfile.h"

#include <QCursor>
#include <QTextOption>
#include <QDebug>
#include <QRgb>

#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>

namespace {
const QSize BACKGROUND_SIZE = QSize(120, 120);
const int SCALE_VALUE = 4;
const int IMG_WIDTH =  55;
const int INDICATOR_WIDTH = 110;
const int CENTER_RECT_WIDTH = 12;
const int BOTTOM_RECT_HEIGHT = 14;
}
ZoomIndicator::ZoomIndicator(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(BACKGROUND_SIZE);
    setStyleSheet(getFileContent(":/zoomindicator.qss"));
    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_centerRect = QRect((BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH)/2 + 1,
                             (BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH)/2 + 1,
                             CENTER_RECT_WIDTH, CENTER_RECT_WIDTH);
    fullscreenImgFile = QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
}

ZoomIndicator::~ZoomIndicator() {}


void ZoomIndicator::paintEvent(QPaintEvent *) {
//    using namespace utils;
    QPoint centerPos =  this->cursor().pos();
    centerPos = QPoint(std::max(centerPos.x() - this->window()->x(), 0),
                           std::max(centerPos.y() - this->window()->y(), 0));

    QPainter painter(this);

    //QPixmap fullscreenImgFile = QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());

    //const QPixmap &fullscreenImgFile = TempFile::instance()->getFullscreenPixmap();

    qreal ration = this->devicePixelRatioF();
    QImage fullscreenImg = fullscreenImgFile.toImage();
    fullscreenImg =  fullscreenImg.scaled(fullscreenImg.width()/ration,
                                          fullscreenImg.height()/ration, Qt::KeepAspectRatio);
    const QRgb centerRectRgb = fullscreenImg.pixel(centerPos);
    QPixmap zoomPix;
    int indicator_width = INDICATOR_WIDTH;
    int indicator_height = INDICATOR_WIDTH;
    int x;
    int y;

    if(centerPos.x() > fullscreenImg.width() - IMG_WIDTH/2  && centerPos.y() > fullscreenImg.height() - IMG_WIDTH/2)
    {
        zoomPix = QPixmap(fullscreenImgFile).scaled(
                    fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
                    centerPos.x() - IMG_WIDTH/2, centerPos.y() - IMG_WIDTH/2,
                    IMG_WIDTH/2 + fullscreenImg.width() - centerPos.x(), IMG_WIDTH/2 + fullscreenImg.height() - centerPos.y());

        indicator_width = INDICATOR_WIDTH - 2*(IMG_WIDTH/2 + centerPos.x() - fullscreenImg.width())*(INDICATOR_WIDTH/IMG_WIDTH);
        indicator_height = INDICATOR_WIDTH - 2*(IMG_WIDTH/2 + centerPos.y() - fullscreenImg.height())*(INDICATOR_WIDTH/IMG_WIDTH);


        zoomPix = zoomPix.scaled(QSize(indicator_width,  indicator_height),
                                 Qt::KeepAspectRatio);
        m_lastPic = zoomPix;

        painter.drawPixmap(QRect(5, 5, indicator_width, indicator_height), zoomPix);
    }
    else if(centerPos.x() <  IMG_WIDTH/2)
    {
        zoomPix = QPixmap(fullscreenImgFile).scaled(
                    fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
                    centerPos.x() , centerPos.y() - IMG_WIDTH/2,
                    IMG_WIDTH/2 + centerPos.x(), IMG_WIDTH);

        indicator_width = INDICATOR_WIDTH - (IMG_WIDTH/2-centerPos.x())*(INDICATOR_WIDTH/IMG_WIDTH);
        //indicator_height = INDICATOR_WIDTH - (IMG_WIDTH/2 - centerPos.y())*(INDICATOR_WIDTH/IMG_WIDTH);
        x = 2*(IMG_WIDTH/2 - centerPos.x());
        //y = 2*(IMG_WIDTH/2 - centerPos.y());

        zoomPix = zoomPix.scaled(QSize(indicator_width,  indicator_height),
                                 Qt::KeepAspectRatio).copy(
                    centerPos.x() , centerPos.y() - IMG_WIDTH/2,
                    indicator_width, indicator_height);

        m_lastPic = zoomPix;

        painter.drawPixmap(QRect(5 + x, 5, indicator_width, indicator_height), zoomPix);
    }
    else if(centerPos.x() > fullscreenImg.width() - IMG_WIDTH/2)
    {
         zoomPix = QPixmap(fullscreenImgFile).scaled(
                    fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
                    centerPos.x() - IMG_WIDTH/2, centerPos.y() - IMG_WIDTH/2,
                    IMG_WIDTH/2 + fullscreenImg.width() - centerPos.x(), IMG_WIDTH);

         indicator_width = INDICATOR_WIDTH - 2*(IMG_WIDTH/2 + centerPos.x() - fullscreenImg.width())*(INDICATOR_WIDTH/IMG_WIDTH);

         zoomPix = zoomPix.scaled(QSize(indicator_width,  indicator_height),
                                  Qt::KeepAspectRatio);
         m_lastPic = zoomPix;

         painter.drawPixmap(QRect(5, 5, indicator_width, indicator_height), zoomPix);
    }
    else if(centerPos.y() > fullscreenImg.height() - IMG_WIDTH/2)
    {
         zoomPix = QPixmap(fullscreenImgFile).scaled(
                    fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
                    centerPos.x() - IMG_WIDTH/2, centerPos.y() - IMG_WIDTH/2,
                    IMG_WIDTH, IMG_WIDTH/2 + fullscreenImg.height() - centerPos.y());
         indicator_height = INDICATOR_WIDTH - 2*(IMG_WIDTH/2 + centerPos.y() - fullscreenImg.height())*(INDICATOR_WIDTH/IMG_WIDTH);

         zoomPix = zoomPix.scaled(QSize(indicator_width,  indicator_height),
                                  Qt::KeepAspectRatio);
         m_lastPic = zoomPix;

         painter.drawPixmap(QRect(5, 5, indicator_width, indicator_height), zoomPix);
    }
    else
    {
         zoomPix = QPixmap(fullscreenImgFile).scaled(
                    fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
                    centerPos.x() - IMG_WIDTH/2, centerPos.y() - IMG_WIDTH/2,
                    IMG_WIDTH, IMG_WIDTH);

         zoomPix = zoomPix.scaled(QSize(indicator_width,  indicator_height),
                                  Qt::KeepAspectRatio);
         m_lastPic = zoomPix;


         painter.drawPixmap(QRect(5, 5, indicator_width, indicator_height), zoomPix);
    }
   /* zoomPix = QPixmap(fullscreenImgFile).scaled(
               fullscreenImg.width(), fullscreenImg.height(),Qt::KeepAspectRatio).copy(
               centerPos.x() - IMG_WIDTH/2, centerPos.y() - IMG_WIDTH/2,
               IMG_WIDTH, IMG_WIDTH);*/


    painter.drawPixmap(m_centerRect, QPixmap(":/center_rect.png"));
    m_lastCenterPosBrush = QBrush(QColor(qRed(centerRectRgb),
                                         qGreen(centerRectRgb), qBlue(centerRectRgb)));
    painter.fillRect(QRect(INDICATOR_WIDTH/2 + 2, INDICATOR_WIDTH/2 + 2,
                           CENTER_RECT_WIDTH - 4, CENTER_RECT_WIDTH - 4), m_lastCenterPosBrush);

    painter.fillRect(QRect(5, INDICATOR_WIDTH - 9, INDICATOR_WIDTH, BOTTOM_RECT_HEIGHT),
                     QBrush(QColor(0, 0, 0, 125)));

    QPen pen;
    pen.setBrush(QBrush(Qt::green));
    QVector<qreal> dashes;
    qreal space = 4;
    dashes << 1 << space << 9 << space << 27 << space;
    pen.setDashPattern(dashes);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.drawLine(5+1+INDICATOR_WIDTH/2, 5, 5+1+INDICATOR_WIDTH/2, 5+INDICATOR_WIDTH);

    painter.drawLine(5, 5+1+INDICATOR_WIDTH/2, 5+INDICATOR_WIDTH, 5+1+INDICATOR_WIDTH/2);

    QFont posFont;
    posFont.setPixelSize(9);
    painter.setFont(posFont);
    painter.setPen(QColor(Qt::white));
    QTextOption posTextOption;
    posTextOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    painter.drawText(QRectF(5, INDICATOR_WIDTH - 10, INDICATOR_WIDTH, INDICATOR_WIDTH),
                     QString("%1, %2").arg(centerPos.x()).arg(centerPos.y()), posTextOption);
}

void ZoomIndicator::showMagnifier(QPoint pos) {
    this->show();

   this->move(pos);
}
