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

#include <QList>
#include <QMenu>
#include <QObject>
#include <QPoint>

enum SaveAction : unsigned int;

// 编辑器保存按钮弹出的菜单：选择保存位置与图片格式。
//
// 它只维护菜单状态并对外发信号，不执行任何保存动作——落盘由编辑器窗口负责，
// 这与旧行为一致。菜单结构与可见文案保持不变。
class SaveMenuController : public QObject
{
    Q_OBJECT

public:
    explicit SaveMenuController(QObject *parent = 0);
    ~SaveMenuController();

    QMenu *getMenu();

signals:
    void shapePressed(QString currentShape);
    void unDoAction();
    void saveBtnPressed(SaveAction action);
    void menuNoFocus();
    void setPformat(QString format);
    void setSaveTip(QString saveTip);
    void menuHide();

public slots:
    void showMenu(QPoint pos);
    void hide();
    void setDefaultFormat(QString pformat);
    void setDefaultCheck(QString saveTip);

private slots:
    // 两组选项各自共用一个槽：具体选了哪一项由触发动作携带的 data 决定，
    // 这样不必为每个菜单项维护一份几乎相同的处理函数。
    void onPathOptionTriggered();
    void onFormatOptionTriggered();

private:
    QList<QAction *> pathOptions() const;
    QList<QAction *> formatOptions() const;

    QMenu *m_menu;

    QAction *m_pathHeader;
    QAction *m_desktopOption;
    QAction *m_clipboardOption;
    QAction *m_customOption;

    QAction *m_formatHeader;
    QAction *m_jpgOption;
    QAction *m_pngOption;
    QAction *m_bmpOption;
};
