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

#include "src/tools/text/textsizebutton.h"

#include "src/utils/captureutils.h"

#include <QEvent>

namespace
{

// 下拉项顺序固定为 small / medium / large，这里是它与尺寸常量的唯一映射处。
int indexOfThickness(int thickness)
{
    if (thickness == captureutils::TextSizeMedium) {
        return 1;
    }
    if (thickness == captureutils::TextSizeLarge) {
        return 2;
    }
    return 0;
}

int thicknessOfIndex(int index)
{
    if (index == 1) {
        return captureutils::TextSizeMedium;
    }
    if (index == 2) {
        return captureutils::TextSizeLarge;
    }
    return captureutils::TextSizeSmall;
}

}  // namespace

TextSizeButton::TextSizeButton(int thickness, QWidget *parent)
    : QComboBox(parent), m_thickness(thickness)
{
    setObjectName(QStringLiteral("TextSizeButton"));
    addItem(tr("small"));
    addItem(tr("medium"));
    addItem(tr("large"));
    setCurrentIndex(indexOfThickness(thickness));
    setFixedSize(24, 26);
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setThickness(int)));
}

int TextSizeButton::thickness() const
{
    return m_thickness;
}

void TextSizeButton::setTips(QString tips)
{
    m_tips = tips;
}

QString TextSizeButton::getTips()
{
    return m_tips;
}

void TextSizeButton::setThickness(int index)
{
    Q_UNUSED(index);
    m_thickness = thicknessOfIndex(currentIndex());
    emit updateThickness(m_thickness);
}

void TextSizeButton::enterEvent(QEvent *e)
{
    emit onEnter();
    QComboBox::enterEvent(e);
}

void TextSizeButton::leaveEvent(QEvent *e)
{
    emit onExist();
    QComboBox::leaveEvent(e);
}
