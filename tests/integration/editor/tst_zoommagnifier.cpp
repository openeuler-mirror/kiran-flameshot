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

// ZoomMagnifier 测试，分两层：
//   - 边缘翻转与裁剪区域是纯几何运算，直接断言返回值；
//   - 绘制结果必须驱动真实控件，这里把真实 ZoomMagnifier 渲染到 QImage。
//
// 控件在构造期取一次整屏快照，因此本用例需要真实 X server；由
// run-legacy-editor.sh 提供 Xvfb，没有 xvfb-run 时整体记为 Skipped。

#include <QtTest>

#include <QApplication>
#include <QImage>

#include "src/widgets/capture/zoommagnifier.h"

namespace {

const QSize kScreen(800, 600);
// 46 / 2；边界判定与裁剪都基于它。
const int kHalf = 23;

}  // namespace

class TestZoomMagnifier : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void centerKeepsFullSource();
    void rightEdgeShrinksWidth();
    void leftEdgeShiftsTargetRight();
    void bottomEdgeShrinksHeight();
    void bottomRightCornerShrinksBothSides();
    void rendersRealWidget();
};

void TestZoomMagnifier::initTestCase()
{
    if (QApplication::primaryScreen() == 0) {
        QSKIP("没有可用的屏幕。");
    }
}

void TestZoomMagnifier::centerKeepsFullSource()
{
    const zoommagnifier::Placement placement =
        zoommagnifier::placementFor(QPoint(400, 300), kScreen);

    QCOMPARE(placement.source, QRect(400 - kHalf, 300 - kHalf, 46, 46));
    QCOMPARE(placement.target, QRect(5, 5, 89, 89));
    QVERIFY(placement.secondClip.isNull());
}

void TestZoomMagnifier::rightEdgeShrinksWidth()
{
    const QPoint center(790, 300);
    const zoommagnifier::Placement placement = zoommagnifier::placementFor(center, kScreen);

    QCOMPARE(placement.source,
             QRect(center.x() - kHalf, center.y() - kHalf,
                   kHalf + kScreen.width() - center.x(), 46));
    // 89 - 2 * (23 + 790 - 800) == 63
    QCOMPARE(placement.target.size(), QSize(63, 89));
    QCOMPARE(placement.target.topLeft(), QPoint(5, 5));
    QVERIFY(placement.secondClip.isNull());
}

void TestZoomMagnifier::leftEdgeShiftsTargetRight()
{
    const QPoint center(10, 300);
    const zoommagnifier::Placement placement = zoommagnifier::placementFor(center, kScreen);

    QCOMPARE(placement.source, QRect(center.x(), center.y() - kHalf, kHalf + center.x(), 46));
    // 89 - (23 - 10) == 76，并且整体右移 2 * (23 - 10) == 26
    QCOMPARE(placement.target, QRect(5 + 26, 5, 76, 89));
    QVERIFY(!placement.secondClip.isNull());
    QCOMPARE(placement.secondClip.width(), 76);
}

void TestZoomMagnifier::bottomEdgeShrinksHeight()
{
    const QPoint center(400, 590);
    const zoommagnifier::Placement placement = zoommagnifier::placementFor(center, kScreen);

    QCOMPARE(placement.source,
             QRect(center.x() - kHalf, center.y() - kHalf,
                   46, kHalf + kScreen.height() - center.y()));
    // 89 - 2 * (23 + 590 - 600) == 63
    QCOMPARE(placement.target.size(), QSize(89, 63));
    QVERIFY(placement.secondClip.isNull());
}

void TestZoomMagnifier::bottomRightCornerShrinksBothSides()
{
    const QPoint center(790, 590);
    const zoommagnifier::Placement placement = zoommagnifier::placementFor(center, kScreen);

    QCOMPARE(placement.source,
             QRect(center.x() - kHalf, center.y() - kHalf,
                   kHalf + kScreen.width() - center.x(),
                   kHalf + kScreen.height() - center.y()));
    QCOMPARE(placement.target.size(), QSize(63, 63));
    QVERIFY(placement.secondClip.isNull());
}

void TestZoomMagnifier::rendersRealWidget()
{
    ZoomMagnifier magnifier;
    QCOMPARE(magnifier.size(), QSize(99, 99));

    magnifier.showMagnifier(QPoint(120, 120));
    QVERIFY(magnifier.isVisible());

    QImage canvas(magnifier.size(), QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    magnifier.render(&canvas);

    QCOMPARE(canvas.size(), QSize(99, 99));
    // 放大内容来自真实屏幕快照，因此渲染结果不应是整块透明。
    bool sawPixel = false;
    for (int y = 0; y < canvas.height() && !sawPixel; ++y) {
        for (int x = 0; x < canvas.width(); ++x) {
            if (qAlpha(canvas.pixel(x, y)) != 0) {
                sawPixel = true;
                break;
            }
        }
    }
    QVERIFY(sawPixel);
}

QTEST_MAIN(TestZoomMagnifier)

#include "tst_zoommagnifier.moc"
