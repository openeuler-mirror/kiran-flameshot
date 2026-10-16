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

#include "src/widgets/capture/savemenucontroller.h"

#include "src/utils/saveutils.h"

#include <QAction>
#include <QStyleFactory>
#include <QString>

namespace
{

// 保存位置与格式的菜单项标识。它们同时作为 QAction::data，触发时直接回传，
// 因此字符串必须与窗口侧读取的 savetip / 图片格式取值一致。
const char kDesktopTip[] = "desktip";
const char kClipboardTip[] = "clicktip";
const char kCustomTip[] = "customtip";
const char kJpgFormat[] = "jpg";
const char kPngFormat[] = "png";
const char kBmpFormat[] = "bmp";

QString menuStyleSheet()
{
    return QStringLiteral(
        "QMenu{background-color: rgb(255, 255, 255);"
        "border-radius: 20px; "
        "color:#222222;"
        "}"
        "QMenu::item { background-color: transparent;"
        "border-radius: 20px; "
        "padding:3px 20px 3px 24px;"
        "margin:5px 10px;"
        "}"
        "QMenu::item:selected { background-color: gray; }");
}

QAction *optionWithValue(const QList<QAction *> &options, const QString &value)
{
    for (int i = 0; i < options.size(); ++i) {
        if (options.at(i)->data().toString() == value) {
            return options.at(i);
        }
    }
    return 0;
}

}  // namespace

SaveMenuController::SaveMenuController(QObject *parent)
    : QObject(parent),
      m_menu(new QMenu),
      m_pathHeader(new QAction(tr("save to"))),
      m_desktopOption(new QAction(tr("desktop"))),
      m_clipboardOption(new QAction(tr("cilickborad"))),
      m_customOption(new QAction(tr("Custom save location"))),
      m_formatHeader(new QAction(tr("Save format"))),
      m_jpgOption(new QAction(tr("JPG"))),
      m_pngOption(new QAction(tr("PNG"))),
      m_bmpOption(new QAction(tr("BMP")))
{
    m_menu->setFocusPolicy(Qt::StrongFocus);
    m_menu->setStyle(QStyleFactory::create(QStringLiteral("dlight")));
    m_menu->setStyleSheet(menuStyleSheet());

    m_desktopOption->setData(QString::fromLatin1(kDesktopTip));
    m_clipboardOption->setData(QString::fromLatin1(kClipboardTip));
    m_customOption->setData(QString::fromLatin1(kCustomTip));
    m_jpgOption->setData(QString::fromLatin1(kJpgFormat));
    m_pngOption->setData(QString::fromLatin1(kPngFormat));
    m_bmpOption->setData(QString::fromLatin1(kBmpFormat));

    const QList<QAction *> paths = pathOptions();
    for (int i = 0; i < paths.size(); ++i) {
        paths.at(i)->setCheckable(true);
    }
    const QList<QAction *> formats = formatOptions();
    for (int i = 0; i < formats.size(); ++i) {
        formats.at(i)->setCheckable(true);
    }

    // 两个分组标题只起说明作用，不可点击。
    m_pathHeader->setEnabled(false);
    m_formatHeader->setEnabled(false);

    m_menu->addAction(m_pathHeader);
    m_menu->addAction(m_desktopOption);
    m_menu->addAction(m_clipboardOption);
    m_menu->addAction(m_customOption);
    m_menu->addAction(m_formatHeader);
    m_menu->addAction(m_jpgOption);
    m_menu->addAction(m_pngOption);
    m_menu->addAction(m_bmpOption);

    connect(m_menu, &QMenu::aboutToHide, this, &SaveMenuController::menuHide);

    for (int i = 0; i < paths.size(); ++i) {
        connect(paths.at(i), &QAction::triggered,
                this, &SaveMenuController::onPathOptionTriggered);
    }
    for (int i = 0; i < formats.size(); ++i) {
        connect(formats.at(i), &QAction::triggered,
                this, &SaveMenuController::onFormatOptionTriggered);
    }
}

SaveMenuController::~SaveMenuController()
{
}

QList<QAction *> SaveMenuController::pathOptions() const
{
    return QList<QAction *>() << m_desktopOption << m_clipboardOption << m_customOption;
}

QList<QAction *> SaveMenuController::formatOptions() const
{
    return QList<QAction *>() << m_jpgOption << m_pngOption << m_bmpOption;
}

QMenu *SaveMenuController::getMenu()
{
    return m_menu;
}

void SaveMenuController::showMenu(QPoint pos)
{
    m_menu->popup(pos);
}

void SaveMenuController::hide()
{
    m_menu->hide();
}

void SaveMenuController::onPathOptionTriggered()
{
    QAction *triggered = qobject_cast<QAction *>(sender());
    if (triggered == 0) {
        return;
    }

    // 保存位置是单选：勾中的那一项保留，另外两项取消。
    const QList<QAction *> options = pathOptions();
    for (int i = 0; i < options.size(); ++i) {
        options.at(i)->setChecked(options.at(i) == triggered);
    }
    emit setSaveTip(triggered->data().toString());
}

void SaveMenuController::onFormatOptionTriggered()
{
    QAction *triggered = qobject_cast<QAction *>(sender());
    if (triggered == 0) {
        return;
    }

    const QList<QAction *> options = formatOptions();
    for (int i = 0; i < options.size(); ++i) {
        options.at(i)->setChecked(options.at(i) == triggered);
    }
    emit setPformat(triggered->data().toString());
}

void SaveMenuController::setDefaultFormat(QString pformat)
{
    // 与历史行为一致：只勾选匹配项，不主动清掉其它项。
    QAction *match = optionWithValue(formatOptions(), pformat);
    if (match != 0) {
        match->setChecked(true);
    }
}

void SaveMenuController::setDefaultCheck(QString saveTip)
{
    QAction *match = optionWithValue(pathOptions(), saveTip);
    if (match != 0) {
        match->setChecked(true);
    }
}
