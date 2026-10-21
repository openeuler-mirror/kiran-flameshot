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

#include "src/widgets/capture/selectionhintlabel.h"

#include "src/utils/captureutils.h"

#include <QString>

namespace
{

const int kHintWidth = 75;
const int kHintHeight = 20;
// 提示与选区边界的间距。
const int kHintGap = 3;

// 提示默认贴在光标下方；当光标位置已经超过标签高度时改为贴在上方。
// 这段几何不依赖 widget 状态，单独成函数便于直接验证。
QPoint hintPositionFor(const QPoint &cursor, const QSize &hintSize)
{
    const int yOffset = cursor.y() > hintSize.height()
                            ? cursor.y() - hintSize.height() - kHintGap
                            : cursor.y() + kHintGap;
    return QPoint(cursor.x(), yOffset);
}

}  // namespace

SelectionHintLabel::SelectionHintLabel(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(kHintWidth, kHintHeight);
    // 样式表放在 qss 资源里，与放大镜一致；其中用的是类名选择器，改名必须同步。
    setStyleSheet(captureutils::getFileContent(QStringLiteral(":/qss/selectionhintlabel.qss")));
}

SelectionHintLabel::~SelectionHintLabel()
{
}

void SelectionHintLabel::setContent(QString widthXHeight)
{
    QLabel::setText(widthXHeight);
    QLabel::setAlignment(Qt::AlignCenter);
}

void SelectionHintLabel::updateTips(QPoint pos, QString text)
{
    if (isHidden()) {
        show();
    }

    move(hintPositionFor(pos, size()));
    setContent(text);
}
