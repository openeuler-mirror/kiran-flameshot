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

// captureutils 的行为对照测试。
//
// 混合形态是有意的：
//   - 颜色映射、后缀白名单、枚举数值不依赖 Qt 状态，直接断言返回值；
//   - 光标构造、字宽计算、按位置绘制需要真实的 QGuiApplication/QPainter，
//     因此在 offscreen QPA 下用真实对象驱动，不使用任何替身。

#include <QtTest>

#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include "src/utils/captureutils.h"

class TestCaptureUtils : public QObject
{
    Q_OBJECT

private slots:
    void colorIndexOfKnownSlots();
    void colorIndexOfFallsBackOutOfRange();
    void colorIndexMapsOnlyConfiguredSlots();
    void validFormats();
    void numericConstantsStayStable();
    void cursorShapeFallsBackForUnknownName();
    void cursorShapeBuildsCustomCursorForKnownName();
    void stringWidthMatchesFontMetrics();
    void paintSelectedPointDrawsAtRequestedPosition();
};

void TestCaptureUtils::colorIndexOfKnownSlots()
{
    QCOMPARE(captureutils::colorIndexOf(0), QColor(QStringLiteral("#ff1c49")));
    QCOMPARE(captureutils::colorIndexOf(1), QColor(QStringLiteral("#00aaff")));
    QCOMPARE(captureutils::colorIndexOf(2), QColor(QStringLiteral("#08ff77")));
    QCOMPARE(captureutils::colorIndexOf(3), QColor(QStringLiteral("#ffffff")));
    QCOMPARE(captureutils::colorIndexOf(4), QColor(QStringLiteral("#000000")));
}

void TestCaptureUtils::colorIndexOfFallsBackOutOfRange()
{
    // 与历史实现的 default 分支一致：越界与负数都回落为绿色。
    QCOMPARE(captureutils::colorIndexOf(5), QColor(QStringLiteral("#08ff77")));
    QCOMPARE(captureutils::colorIndexOf(-1), QColor(QStringLiteral("#08ff77")));
}

void TestCaptureUtils::colorIndexMapsOnlyConfiguredSlots()
{
    QCOMPARE(captureutils::colorIndex(QColor(QStringLiteral("#00aaff"))), 0);
    QCOMPARE(captureutils::colorIndex(QColor(QStringLiteral("#08ff77"))), 1);
    QCOMPARE(captureutils::colorIndex(QColor(QStringLiteral("#ffffff"))), 2);
    QCOMPARE(captureutils::colorIndex(QColor(QStringLiteral("#000000"))), 3);
    // 索引 0 的红色并不在反向表里，历史实现同样返回 -1，不得"顺手修好"。
    QCOMPARE(captureutils::colorIndex(QColor(QStringLiteral("#ff1c49"))), -1);
}

void TestCaptureUtils::validFormats()
{
    const char *accepted[] = { "bmp", "jpg", "jpeg", "png", "pbm", "pgm", "xbm", "xpm" };
    for (unsigned i = 0; i < sizeof(accepted) / sizeof(accepted[0]); ++i) {
        QVERIFY2(captureutils::isValidFormat(QString::fromLatin1(accepted[i])), accepted[i]);
    }
    QVERIFY(!captureutils::isValidFormat(QStringLiteral("gif")));
    QVERIFY(!captureutils::isValidFormat(QStringLiteral("PNG")));
    QVERIFY(!captureutils::isValidFormat(QString()));
}

void TestCaptureUtils::numericConstantsStayStable()
{
    // 这些数值会被写进用户配置，改动会让已有配置的含义漂移。
    QCOMPARE(captureutils::LineSolid, 1);
    QCOMPARE(captureutils::LineArrow, 2);
    QCOMPARE(captureutils::LineDotted, 3);
    QCOMPARE(captureutils::ShapeRectAngle, 1);
    QCOMPARE(captureutils::ShapeBlur, 2);
    QCOMPARE(captureutils::ShapeCircle, 3);
    QCOMPARE(captureutils::TextSizeSmall, 1);
    QCOMPARE(captureutils::TextSizeMedium, 8);
    QCOMPARE(captureutils::TextSizeLarge, 16);
}

void TestCaptureUtils::cursorShapeFallsBackForUnknownName()
{
    // 未识别的形状名返回默认构造的 QCursor，与历史行为一致。
    QCOMPARE(captureutils::setCursorShape(QStringLiteral("no-such-shape")).shape(),
             QCursor().shape());
}

void TestCaptureUtils::cursorShapeBuildsCustomCursorForKnownName()
{
    // 已知形状构造的是位图光标；这里只断言形状类别，不断言像素内容，
    // 因为图标来自 qrc，内容由资源文件负责。
    QCOMPARE(captureutils::setCursorShape(QStringLiteral("rectangle")).shape(),
             Qt::BitmapCursor);
    QCOMPARE(captureutils::setCursorShape(QStringLiteral("line"), 0).shape(),
             Qt::BitmapCursor);
}

void TestCaptureUtils::stringWidthMatchesFontMetrics()
{
    QFont font;
    font.setPointSize(12);
    const QString text = QStringLiteral("Kiran 截图");
    QCOMPARE(captureutils::stringWidth(font, text),
             QFontMetrics(font).boundingRect(text).width());
}

void TestCaptureUtils::paintSelectedPointDrawsAtRequestedPosition()
{
    QImage canvas(40, 40, QImage::Format_RGB32);
    canvas.fill(Qt::black);

    QPixmap marker(4, 4);
    marker.fill(Qt::red);

    QPainter painter(&canvas);
    captureutils::paintSelectedPoint(painter, QPointF(10, 10), marker);
    painter.end();

    QCOMPARE(canvas.pixelColor(11, 11), QColor(Qt::red));
    QCOMPARE(canvas.pixelColor(5, 5), QColor(Qt::black));
}

QTEST_MAIN(TestCaptureUtils)

#include "tst_captureutils.moc"
