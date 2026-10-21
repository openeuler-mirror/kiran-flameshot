// Copyright (C) 2026 KylinSec Co., Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "src/widgets/capture/zoommagnifier.h"

#include "src/utils/captureutils.h"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QDesktopWidget>
#include <QFont>
#include <QGuiApplication>
#include <QImage>
#include <QPen>
#include <QPixmap>
#include <QRgb>
#include <QScreen>
#include <QTextOption>
#include <QVector>

#include <algorithm>

namespace
{

const int kMagnifierSide = 99;      // 放大镜整体尺寸
const int kIndicatorSide = 89;      // 内部绘制区边长
const int kSourceSide = 46;         // 中间位置时从整屏图裁剪的边长
const int kCenterSide = 12;         // 中心点标记的边长
const int kBottomStripHeight = 14;  // 底部坐标条高度
const int kPaintMargin = 5;         // 绘制区相对放大镜左上角的边距
const int kCursorGap = 3;           // 放大镜与光标的间距

// 历史实现里这个比例是整数除法（89 / 46 == 1）。它参与边缘缩小量的计算，
// 因此这里保持同样的整数语义，不能改成浮点比例。
const int kIndicatorPerSource = kIndicatorSide / kSourceSide;

const int sourceHalfSide()
{
    return kSourceSide / 2;
}

}  // namespace

namespace zoommagnifier
{

Placement placementFor(const QPoint &center, const QSize &screen)
{
    const int half = sourceHalfSide();

    Placement placement;
    placement.target = QRect(kPaintMargin, kPaintMargin, kIndicatorSide, kIndicatorSide);

    const bool touchesRight = center.x() > screen.width() - half;
    const bool touchesLeft = center.x() < half;
    const bool touchesBottom = center.y() > screen.height() - half;

    // 分支顺序必须保持：右下角优先，其次是左右边缘，再到下边缘，最后是中间区域。
    if (touchesRight && touchesBottom) {
        placement.source = QRect(center.x() - half, center.y() - half,
                                 half + screen.width() - center.x(),
                                 half + screen.height() - center.y());
        const int width = kIndicatorSide
                          - 2 * (half + center.x() - screen.width()) * kIndicatorPerSource;
        const int height = kIndicatorSide
                           - 2 * (half + center.y() - screen.height()) * kIndicatorPerSource;
        placement.target.setSize(QSize(width, height));
        return placement;
    }

    if (touchesLeft) {
        placement.source = QRect(center.x(), center.y() - half,
                                 half + center.x(), kSourceSide);
        const int width = kIndicatorSide - (half - center.x()) * kIndicatorPerSource;
        placement.target = QRect(kPaintMargin + 2 * (half - center.x()), kPaintMargin,
                                 width, kIndicatorSide);
        // 这一支在缩放之后还会按屏幕坐标再裁一次，是历史行为的一部分。
        placement.secondClip = QRect(center.x(), center.y() - half, width, kIndicatorSide);
        return placement;
    }

    if (touchesRight) {
        placement.source = QRect(center.x() - half, center.y() - half,
                                 half + screen.width() - center.x(), kSourceSide);
        const int width = kIndicatorSide
                          - 2 * (half + center.x() - screen.width()) * kIndicatorPerSource;
        placement.target.setSize(QSize(width, kIndicatorSide));
        return placement;
    }

    if (touchesBottom) {
        placement.source = QRect(center.x() - half, center.y() - half,
                                 kSourceSide, half + screen.height() - center.y());
        const int height = kIndicatorSide
                           - 2 * (half + center.y() - screen.height()) * kIndicatorPerSource;
        placement.target.setSize(QSize(kIndicatorSide, height));
        return placement;
    }

    placement.source = QRect(center.x() - half, center.y() - half, kSourceSide, kSourceSide);
    return placement;
}

}  // namespace zoommagnifier

ZoomMagnifier::ZoomMagnifier(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(kMagnifierSide, kMagnifierSide);
    setStyleSheet(captureutils::getFileContent(QStringLiteral(":/qss/zoommagnifier.qss")));
    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_centerRect = QRect((kMagnifierSide - kCenterSide) / 2 + 1,
                         (kMagnifierSide - kCenterSide) / 2 + 1,
                         kCenterSide, kCenterSide);

    // 放大镜显示的始终是"整屏快照"，与编辑器窗口自身的重绘无关；这也是它与
    // CaptureWidget 的区别：它不参与交互式抓屏路径，只在打开时取一次桌面。
    if (QApplication::primaryScreen() != 0) {
        m_fullScreenShots =
            QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
    }
}

ZoomMagnifier::~ZoomMagnifier()
{
}

void ZoomMagnifier::showMagnifier(QPoint pos)
{
    show();
    move(pos);
}

void ZoomMagnifier::paintEvent(QPaintEvent *)
{
    // 放大镜窗口通常贴着光标，因此把光标坐标换算成相对放大镜的坐标。
    QPoint cursorPos = cursor().pos();
    const QPoint relative(std::max(cursorPos.x() - window()->x(), 0),
                          std::max(cursorPos.y() - window()->y(), 0));

    QPainter painter(this);

    const qreal ratio = devicePixelRatioF();
    QImage fullScreenImage = m_fullScreenShots.toImage();
    fullScreenImage = fullScreenImage.scaled(fullScreenImage.width() / ratio,
                                             fullScreenImage.height() / ratio,
                                             Qt::KeepAspectRatio);

    const zoommagnifier::Placement placement =
        zoommagnifier::placementFor(relative, fullScreenImage.size());

    const QPixmap scaledScreen = QPixmap(m_fullScreenShots).scaled(
        fullScreenImage.width(), fullScreenImage.height(), Qt::KeepAspectRatio);
    QPixmap zoomPixmap = scaledScreen.copy(placement.source);
    zoomPixmap = zoomPixmap.scaled(placement.target.size(), Qt::KeepAspectRatio);
    if (!placement.secondClip.isNull()) {
        zoomPixmap = zoomPixmap.copy(placement.secondClip);
    }
    painter.drawPixmap(placement.target, zoomPixmap);

    painter.drawPixmap(m_centerRect, QPixmap(QStringLiteral(":/img/action/center_rect.png")));

    // 光标正下方的像素颜色显示在放大镜中心，方便取色时对齐。
    const QRgb centerRgb = fullScreenImage.pixel(relative);
    painter.fillRect(QRect(kIndicatorSide / 2 + 2, kIndicatorSide / 2 + 2,
                           kCenterSide - 4, kCenterSide - 4),
                     QBrush(QColor(qRed(centerRgb), qGreen(centerRgb), qBlue(centerRgb))));

    painter.fillRect(QRect(kPaintMargin, kIndicatorSide - 9, kIndicatorSide, kBottomStripHeight),
                     QBrush(QColor(0, 0, 0, 125)));

    QPen crossPen;
    crossPen.setBrush(QBrush(Qt::green));
    QVector<qreal> dashes;
    const qreal space = 4;
    dashes << 1 << space << 9 << space << 27 << space;
    crossPen.setDashPattern(dashes);
    crossPen.setStyle(Qt::DashLine);
    crossPen.setWidth(1);
    painter.setPen(crossPen);

    const int crossCenter = kPaintMargin + 1 + kIndicatorSide / 2;
    painter.drawLine(crossCenter, kPaintMargin,
                     crossCenter, kPaintMargin + kIndicatorSide);
    painter.drawLine(kPaintMargin, crossCenter,
                     kPaintMargin + kIndicatorSide, crossCenter);

    QFont positionFont;
    positionFont.setPixelSize(9);
    painter.setFont(positionFont);
    painter.setPen(QColor(Qt::white));
    QTextOption positionTextOption;
    positionTextOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    painter.drawText(QRectF(kPaintMargin, kIndicatorSide - 10, kIndicatorSide, kIndicatorSide),
                     QStringLiteral("%1, %2").arg(relative.x()).arg(relative.y()),
                     positionTextOption);
}
