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

#include <QComboBox>

// 文字工具的三档尺寸下拉框。
//
// 它只负责把用户的选择翻译成 captureutils::TextSize* 之一，并通过
// updateThickness() 通知取色面板；绘制逻辑不在这里。
class TextSizeButton : public QComboBox
{
    Q_OBJECT

public:
    explicit TextSizeButton(int thickness, QWidget *parent = 0);

    int thickness() const;

public slots:
    void setTips(QString tips);
    QString getTips();
    // 参数仅为兼容 Qt 的 currentIndexChanged(int) 连接；实际取值始终来自
    // 当前下拉索引，与历史行为一致。
    void setThickness(int index);

signals:
    void onEnter();
    void onExist();
    void updateThickness(int thickness);

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;

private:
    int m_thickness;
    QString m_tips;
};
