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

// SelectionHintLabel 的组件测试。
//
// 在真实父 widget 中实例化真实标签并驱动真实的 updateTips 几何，不使用替身。

#include <QtTest>

#include <QWidget>

#include "src/widgets/capture/selectionhintlabel.h"

class TestSelectionHintLabel : public QObject
{
    Q_OBJECT

private slots:
    void contentIsStoredAndCentered();
    void updateTipsPlacesLabelBelowForSmallY();
    void updateTipsPlacesLabelAboveForLargeY();
    void updateTipsShowsLabel();
    void emptyContentKeepsLabelAlive();
};

void TestSelectionHintLabel::contentIsStoredAndCentered()
{
    QWidget parent;
    parent.resize(400, 300);
    SelectionHintLabel label(&parent);

    QCOMPARE(label.size(), QSize(75, 20));
    // 样式来自 qss 资源；为空说明资源没链接进来，背景图会一起消失。
    QVERIFY(!label.styleSheet().isEmpty());

    label.setContent(QStringLiteral("200x120"));
    QCOMPARE(label.text(), QStringLiteral("200x120"));
    QCOMPARE(label.alignment(), Qt::AlignCenter);
}

void TestSelectionHintLabel::updateTipsPlacesLabelBelowForSmallY()
{
    QWidget parent;
    parent.resize(400, 300);
    SelectionHintLabel label(&parent);

    // pos.y() <= height()：提示落在光标下方 3 像素。
    label.updateTips(QPoint(40, 10), QStringLiteral("10x10"));
    QCOMPARE(label.x(), 40);
    QCOMPARE(label.y(), 10 + 3);
}

void TestSelectionHintLabel::updateTipsPlacesLabelAboveForLargeY()
{
    QWidget parent;
    parent.resize(400, 300);
    SelectionHintLabel label(&parent);

    // pos.y() > height()：提示翻到光标上方，并留出 3 像素间距。
    label.updateTips(QPoint(120, 100), QStringLiteral("80x60"));
    QCOMPARE(label.x(), 120);
    QCOMPARE(label.y(), 100 - 20 - 3);
    QCOMPARE(label.text(), QStringLiteral("80x60"));
}

void TestSelectionHintLabel::updateTipsShowsLabel()
{
    QWidget parent;
    parent.resize(400, 300);
    parent.show();
    SelectionHintLabel label(&parent);
    label.hide();
    QVERIFY(!label.isVisible());

    label.updateTips(QPoint(50, 60), QStringLiteral("30x30"));
    QVERIFY(label.isVisible());
}

void TestSelectionHintLabel::emptyContentKeepsLabelAlive()
{
    QWidget parent;
    parent.resize(400, 300);
    SelectionHintLabel label(&parent);

    // 与历史行为一致：setContent 只改文本，不改可见性。
    label.setContent(QString());
    QCOMPARE(label.text(), QString());
}

QTEST_MAIN(TestSelectionHintLabel)

#include "tst_selectionhintlabel.moc"
