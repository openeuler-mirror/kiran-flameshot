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

// TextSizeButton 的组件测试。
//
// 在 offscreen QPA 下实例化真实控件并用真实事件驱动，不 mock 控件、不引入替身。

#include <QtTest>

#include <QApplication>

#include "src/tools/text/textsizebutton.h"
#include "src/utils/captureutils.h"

class TestTextSizeButton : public QObject
{
    Q_OBJECT

private slots:
    void initialIndexMatchesThickness();
    void unknownThicknessFallsBackToSmall();
    void changingIndexEmitsUpdateThickness();
    void tipsRoundTrip();
    void enterAndLeaveEmitSignals();
};

void TestTextSizeButton::initialIndexMatchesThickness()
{
    TextSizeButton small(captureutils::TextSizeSmall);
    QCOMPARE(small.currentIndex(), 0);
    QCOMPARE(small.thickness(), captureutils::TextSizeSmall);
    QCOMPARE(small.count(), 3);

    TextSizeButton medium(captureutils::TextSizeMedium);
    QCOMPARE(medium.currentIndex(), 1);
    QCOMPARE(medium.thickness(), captureutils::TextSizeMedium);

    TextSizeButton large(captureutils::TextSizeLarge);
    QCOMPARE(large.currentIndex(), 2);
    QCOMPARE(large.thickness(), captureutils::TextSizeLarge);
}

void TestTextSizeButton::unknownThicknessFallsBackToSmall()
{
    // 历史实现只识别三档尺寸，其余取值停留在默认下拉项。
    TextSizeButton button(42);
    QCOMPARE(button.currentIndex(), 0);
}

void TestTextSizeButton::changingIndexEmitsUpdateThickness()
{
    TextSizeButton button(captureutils::TextSizeSmall);
    QSignalSpy spy(&button, &TextSizeButton::updateThickness);
    QVERIFY(spy.isValid());

    button.setCurrentIndex(2);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), captureutils::TextSizeLarge);
    QCOMPARE(button.thickness(), captureutils::TextSizeLarge);

    button.setCurrentIndex(1);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toInt(), captureutils::TextSizeMedium);
}

void TestTextSizeButton::tipsRoundTrip()
{
    TextSizeButton button(captureutils::TextSizeSmall);
    QCOMPARE(button.getTips(), QString());

    button.setTips(QStringLiteral("thickness"));
    QCOMPARE(button.getTips(), QStringLiteral("thickness"));
}

void TestTextSizeButton::enterAndLeaveEmitSignals()
{
    TextSizeButton button(captureutils::TextSizeSmall);
    QSignalSpy enterSpy(&button, &TextSizeButton::onEnter);
    QSignalSpy leaveSpy(&button, &TextSizeButton::onExist);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);
    QCOMPARE(enterSpy.count(), 1);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);
    QCOMPARE(leaveSpy.count(), 1);
}

QTEST_MAIN(TestTextSizeButton)

#include "tst_textsizebutton.moc"
