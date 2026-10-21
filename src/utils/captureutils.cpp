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

#include "src/utils/captureutils.h"

#include <QApplication>
#include <QFile>
#include <QFontMetrics>
#include <QIcon>
#include <QProcess>
#include <QSize>
#include <QStringList>

namespace
{

// 各光标图标的固有尺寸。它们与 SVG 资源本身的画布一致，缩放会破坏热区。
const int kResizeHandleCursorSide = 24;
const int kRectShapeCursorWidth = 22;
const int kRectShapeCursorHeight = 26;
const int kTextCursorWidth = 11;
const int kTextCursorHeight = 23;
const int kColorPenCursorSide = 25;
const int kStartCursorSide = 15;

// 光标热区（相对图标左上角的像素位置），最终会按 DPR 放大。
struct CursorGeometry
{
    QSize iconSize;
    QPoint hotSpot;
};

QString shapeResourceName(const QString &cursorName)
{
    if (cursorName == QLatin1String("start")) {
        return QStringLiteral(":/img/mouse_style/shape/start_mouse.svg");
    }
    if (cursorName == QLatin1String("rotate")) {
        return QStringLiteral(":/img/mouse_style/shape/rotate_mouse.svg");
    }
    if (cursorName == QLatin1String("rectangle")) {
        return QStringLiteral(":/img/mouse_style/shape/rect_mouse.svg");
    }
    if (cursorName == QLatin1String("oval")) {
        return QStringLiteral(":/img/mouse_style/shape/ellipse_mouse.svg");
    }
    if (cursorName == QLatin1String("arrow")) {
        return QStringLiteral(":/img/mouse_style/shape/arrow_mouse.svg");
    }
    if (cursorName == QLatin1String("text")) {
        return QStringLiteral(":/img/mouse_style/shape/text_mouse.svg");
    }
    if (cursorName == QLatin1String("straightLine")) {
        return QStringLiteral(":/img/mouse_style/shape/line_mouse.svg");
    }
    return QString();
}

CursorGeometry geometryOf(const QString &cursorName, int colorIndex)
{
    CursorGeometry geometry;
    if (cursorName == QLatin1String("start")) {
        geometry.iconSize = QSize(kStartCursorSide, kStartCursorSide);
        geometry.hotSpot = QPoint(8, 8);
    } else if (cursorName == QLatin1String("rotate")) {
        geometry.iconSize = QSize(kResizeHandleCursorSide, kResizeHandleCursorSide);
        geometry.hotSpot = QPoint(10, 10);
    } else if (cursorName == QLatin1String("rectangle") || cursorName == QLatin1String("oval")) {
        geometry.iconSize = QSize(kRectShapeCursorWidth, kRectShapeCursorHeight);
        geometry.hotSpot = QPoint(0, 1);
    } else if (cursorName == QLatin1String("arrow")) {
        geometry.iconSize = QSize(kResizeHandleCursorSide, kResizeHandleCursorSide);
        geometry.hotSpot = QPoint(5, 5);
    } else if (cursorName == QLatin1String("text")) {
        geometry.iconSize = QSize(kTextCursorWidth, kTextCursorHeight);
        geometry.hotSpot = QPoint(5, 5);
    } else if (cursorName == QLatin1String("line")) {
        geometry.iconSize = QSize(kColorPenCursorSide, kColorPenCursorSide);
        geometry.hotSpot = QPoint(5, 22);
    } else if (cursorName == QLatin1String("straightLine")) {
        geometry.iconSize = QSize(kResizeHandleCursorSide, kResizeHandleCursorSide);
        geometry.hotSpot = QPoint(2, 9);
    }
    Q_UNUSED(colorIndex);
    return geometry;
}

}  // namespace

namespace captureutils
{

QCursor setCursorShape(QString cursorName, int colorIndex)
{
    const qreal ratio = qApp->devicePixelRatio();

    if (cursorName == QLatin1String("line")) {
        // 取色笔图标按颜色槽位选择，路径里带的编号就是 colorIndex。
        const QString path = QStringLiteral(":/img/mouse_style/color_pen/color%1.svg")
                                 .arg(colorIndex);
        QPixmap pen = QIcon(path).pixmap(QSize(kColorPenCursorSide, kColorPenCursorSide));
        pen.setDevicePixelRatio(ratio);
        return QCursor(pen, static_cast<int>(5 * ratio), static_cast<int>(22 * ratio));
    }

    QString path = shapeResourceName(cursorName);
    if (path.isEmpty()) {
        return QCursor();
    }

    const CursorGeometry geometry = geometryOf(cursorName, colorIndex);

    if (cursorName == QLatin1String("start")) {
        // 起点光标是高 DPI 唯一不按通用规则处理的形状：图标换成 @2x 版本，
        // 但 1x 时热区保持原始像素值，不参与放大。
        if (ratio <= 1) {
            QPixmap pixmap = QIcon(path).pixmap(geometry.iconSize);
            return QCursor(pixmap, geometry.hotSpot.x(), geometry.hotSpot.y());
        }
        path = QStringLiteral(":/img/mouse_style/shape/start_mouse@2x.svg");
    }

    QPixmap pixmap = QIcon(path).pixmap(geometry.iconSize);
    pixmap.setDevicePixelRatio(ratio);
    return QCursor(pixmap,
                   static_cast<int>(geometry.hotSpot.x() * ratio),
                   static_cast<int>(geometry.hotSpot.y() * ratio));
}

int stringWidth(const QFont &f, const QString &str)
{
    return QFontMetrics(f).boundingRect(str).width();
}

QString getFileContent(const QString &file)
{
    QFile handle(file);
    if (!handle.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromLatin1(handle.readAll());
}

QColor colorIndexOf(int index)
{
    // 索引 0 是默认红色，1..4 依次为蓝、绿、白、黑；越界一律回落为绿色，
    // 与历史实现的 default 分支一致。
    const QColor palette[] = {
        QColor(QStringLiteral("#ff1c49")),
        QColor(QStringLiteral("#00aaff")),
        QColor(QStringLiteral("#08ff77")),
        QColor(QStringLiteral("#ffffff")),
        QColor(QStringLiteral("#000000"))
    };
    const int slotCount = static_cast<int>(sizeof(palette) / sizeof(palette[0]));
    if (index < 0 || index >= slotCount) {
        return QColor(QStringLiteral("#08ff77"));
    }
    return palette[index];
}

int colorIndex(QColor color)
{
    // 反向映射只覆盖配置里实际可选的四个颜色；命中不了返回 -1，调用方据此
    // 回落到默认槽位。
    const QColor known[] = {
        QColor(QStringLiteral("#00aaff")),
        QColor(QStringLiteral("#08ff77")),
        QColor(QStringLiteral("#ffffff")),
        QColor(QStringLiteral("#000000"))
    };
    const int knownCount = static_cast<int>(sizeof(known) / sizeof(known[0]));
    for (int slot = 0; slot < knownCount; ++slot) {
        if (known[slot] == color) {
            return slot;
        }
    }
    return -1;
}

bool isValidFormat(QString suffix)
{
    const QStringList supported = QStringList()
        << QStringLiteral("bmp")
        << QStringLiteral("jpg")
        << QStringLiteral("jpeg")
        << QStringLiteral("png")
        << QStringLiteral("pbm")
        << QStringLiteral("pgm")
        << QStringLiteral("xbm")
        << QStringLiteral("xpm");
    return supported.contains(suffix);
}

bool isCommandExist(QString command)
{
    QProcess probe;
    probe.start(QStringLiteral("which"), QStringList() << command);
    probe.waitForFinished(1000);
    return probe.exitCode() == 0;
}

void paintSelectedPoint(QPainter &painter, QPointF pos, QPixmap pointImg)
{
    painter.drawPixmap(pos, pointImg);
}

}  // namespace captureutils
