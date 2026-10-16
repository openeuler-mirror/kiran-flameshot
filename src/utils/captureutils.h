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

#pragma once

// 截图编辑器与选区交互共用的无状态工具。
//
// 这些函数不持有任何状态、不依赖窗口系统，放在一起是因为它们服务同一批调用方
// （选区 widget、取色器、标注工具与放大镜）。它们原先散落在一组历史遗留的工具函数
// 里，本文件按相同职责重新实现，接口语义保持一致。
//
// 约束：本文件只依赖 QtGui/QtCore，不得引入 QWidget 或平台相关类型。

#include <QColor>
#include <QCursor>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QString>

namespace captureutils
{

// 拖拽时鼠标相对选区的位置。取值顺序与历史实现一致：选区做八向缩放，
// Moving 表示整体搬移，Outting 表示落在选区之外。
enum CaptureResizeDirection
{
    Rotate = 0,
    Moving,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Top,
    Bottom,
    Left,
    Right,
    Outting
};

// 画笔线型编号。这些数值会被写进用户配置，因此不可重排。
enum LineStyle
{
    LineSolid = 1,
    LineArrow = 2,
    LineDotted = 3
};

// 标注形状编号。同样是配置值，保持历史取值。
enum ShapeStyle
{
    ShapeRectAngle = 1,
    ShapeBlur = 2,
    ShapeCircle = 3
};

// 文字工具的三档尺寸。数值即选中的字号。
enum TextSize
{
    TextSizeSmall = 1,
    TextSizeMedium = 8,
    TextSizeLarge = 16
};

// 按名称返回自定义鼠标形状；名称不认识时返回 Qt 默认光标。
// colorIndex 只影响 "line" 形状（取色笔图标）。
QCursor setCursorShape(QString cursorName, int colorIndex = 0);

// 文本在给定字体下的像素宽度，用于提示标签的自适应宽度。
int stringWidth(const QFont &f, const QString &str);

// 读取文本文件；打不开时返回空字符串，调用方据此保持原有降级行为。
QString getFileContent(const QString &file);

// 配置里的颜色槽位与 QColor 之间的双向映射。
QColor colorIndexOf(int index);
int colorIndex(QColor color);

// 图片后缀白名单与外部命令探测。
bool isValidFormat(QString suffix);
bool isCommandExist(QString command);

// 在指定位置绘制一个预先生成的小图（放大镜与选区手柄共用）。
void paintSelectedPoint(QPainter &painter, QPointF pos, QPixmap pointImg);

}  // namespace captureutils
