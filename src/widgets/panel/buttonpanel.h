#ifndef BUTTONPANEL_H
#define BUTTONPANEL_H

// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QWidget>
#include <QPointer>
#include <QMenu>


class QVBoxLayout;
class QPropertyAnimation;
class QScrollArea;

class ButtonPanel : public QWidget {
    Q_OBJECT
public:
    explicit ButtonPanel(QWidget *parent = nullptr);

    QWidget* toolWidget() const;
    void addToolWidget(QWidget *w);
    void clearToolWidget();
    void pushWidget(QWidget *w);

signals:
    void mouseEnter();
    void mouseLeave();


private:
    void initInternalPanel();

    QPointer<QWidget> m_toolWidget;
    QScrollArea *m_internalPanel;
    QVBoxLayout *m_upLayout;
    QVBoxLayout *m_layout;
    QPropertyAnimation *m_showAnimation;
    QPropertyAnimation *m_hideAnimation;
    QMenu *m_menu;
};


#endif // BUTTONPANEL_H


