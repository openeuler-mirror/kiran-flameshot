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

// SaveMenuController 的组件测试。
//
// 实例化真实控制器与真实 QMenu，用真实 QAction 触发，断言真实信号与勾选状态，
// 不 mock 菜单、也不替身化信号。

#include <QtTest>

#include <QAction>
#include <QMenu>

#include "src/widgets/capture/savemenucontroller.h"

namespace {

// 菜单顺序在构造里固定：两个分组标题 + 三个保存位置 + 三个格式。
const int kPathHeaderIndex = 0;
const int kDesktopIndex = 1;
const int kClipboardIndex = 2;
const int kCustomIndex = 3;
const int kFormatHeaderIndex = 4;
const int kJpgIndex = 5;
const int kPngIndex = 6;
const int kBmpIndex = 7;

}  // namespace

class TestSaveMenuController : public QObject
{
    Q_OBJECT

private slots:
    void menuHasExpectedEntries();
    void pathOptionEmitsSaveTipAndMarksSingleChoice();
    void formatOptionEmitsPformatAndMarksSingleChoice();
    void defaultSettersCheckMatchingEntry();
    void showAndHideDriveVisibilityAndSignal();
};

void TestSaveMenuController::menuHasExpectedEntries()
{
    SaveMenuController controller;
    QMenu *menu = controller.getMenu();

    QVERIFY(menu != 0);
    QCOMPARE(menu->actions().size(), 8);
    QCOMPARE(menu->actions().at(kPathHeaderIndex)->text(), QStringLiteral("save to"));
    QCOMPARE(menu->actions().at(kFormatHeaderIndex)->text(), QStringLiteral("Save format"));

    // 分组标题不可点击；其余六项都是可勾选的。
    QVERIFY(!menu->actions().at(kPathHeaderIndex)->isEnabled());
    QVERIFY(!menu->actions().at(kFormatHeaderIndex)->isEnabled());
    QVERIFY(menu->actions().at(kDesktopIndex)->isCheckable());
    QVERIFY(menu->actions().at(kBmpIndex)->isCheckable());
}

void TestSaveMenuController::pathOptionEmitsSaveTipAndMarksSingleChoice()
{
    SaveMenuController controller;
    QSignalSpy spy(&controller, &SaveMenuController::setSaveTip);
    QMenu *menu = controller.getMenu();

    menu->actions().at(kDesktopIndex)->trigger();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("desktip"));

    menu->actions().at(kCustomIndex)->trigger();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("customtip"));

    // 保存位置是单选：custom 被选中后 desktop 必须被取消。
    QVERIFY(menu->actions().at(kCustomIndex)->isChecked());
    QVERIFY(!menu->actions().at(kDesktopIndex)->isChecked());
    QVERIFY(!menu->actions().at(kClipboardIndex)->isChecked());
}

void TestSaveMenuController::formatOptionEmitsPformatAndMarksSingleChoice()
{
    SaveMenuController controller;
    QSignalSpy spy(&controller, &SaveMenuController::setPformat);
    QMenu *menu = controller.getMenu();

    menu->actions().at(kJpgIndex)->trigger();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("jpg"));

    menu->actions().at(kBmpIndex)->trigger();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("bmp"));

    QVERIFY(menu->actions().at(kBmpIndex)->isChecked());
    QVERIFY(!menu->actions().at(kJpgIndex)->isChecked());
    QVERIFY(!menu->actions().at(kPngIndex)->isChecked());
}

void TestSaveMenuController::defaultSettersCheckMatchingEntry()
{
    SaveMenuController controller;
    QMenu *menu = controller.getMenu();

    controller.setDefaultFormat(QStringLiteral("png"));
    QVERIFY(menu->actions().at(kPngIndex)->isChecked());

    controller.setDefaultCheck(QStringLiteral("clicktip"));
    QVERIFY(menu->actions().at(kClipboardIndex)->isChecked());

    // 未知取值不改变任何勾选状态。
    controller.setDefaultFormat(QStringLiteral("tiff"));
    controller.setDefaultCheck(QStringLiteral("nowhere"));
    QVERIFY(menu->actions().at(kPngIndex)->isChecked());
    QVERIFY(menu->actions().at(kClipboardIndex)->isChecked());
}

void TestSaveMenuController::showAndHideDriveVisibilityAndSignal()
{
    SaveMenuController controller;
    QMenu *menu = controller.getMenu();
    QSignalSpy hideSpy(&controller, &SaveMenuController::menuHide);

    controller.showMenu(QPoint(20, 20));
    QVERIFY(menu->isVisible());

    controller.hide();
    QVERIFY(!menu->isVisible());
    QCOMPARE(hideSpy.count(), 1);
}

QTEST_MAIN(TestSaveMenuController)

#include "tst_savemenucontroller.moc"
