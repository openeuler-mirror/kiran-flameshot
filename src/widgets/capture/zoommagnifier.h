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

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>

namespace zoommagnifier
{

// 放大镜的放置结果。
//
// source：从整屏图里裁剪的区域，大小随光标靠近屏幕边缘而缩小，保证放大内容不越界。
// target：裁剪结果在放大镜内的绘制区域；靠左时会整体右移，避免超出左侧。
// secondClip：只有"贴左边"这一支在缩放后还需要按屏幕坐标再裁一次，其它分支为空。
struct Placement
{
    QRect source;
    QRect target;
    QRect secondClip;
};

// 该换算是纯几何运算，不依赖任何 widget 状态，因此可以直接单元测试。
Placement placementFor(const QPoint &center, const QSize &screen);

}  // namespace zoommagnifier

// 截图时跟随光标的放大镜。它显示光标附近的放大像素、中心点颜色与光标坐标。
class ZoomMagnifier : public QLabel
{
    Q_OBJECT

public:
    explicit ZoomMagnifier(QWidget *parent = 0);
    ~ZoomMagnifier();

    void showMagnifier(QPoint pos);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QRect m_centerRect;
    QPixmap m_fullScreenShots;
};
