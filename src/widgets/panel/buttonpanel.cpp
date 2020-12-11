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

#include "buttonpanel.h"
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollArea>
#include <QWheelEvent>
#include <QStyleFactory>

//空格呼出页面
ButtonPanel::ButtonPanel(QWidget *parent) : QWidget(parent) {
    initInternalPanel();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setCursor(Qt::ArrowCursor);

}

QWidget *ButtonPanel::toolWidget() const {
    return m_toolWidget;
}

void ButtonPanel::addToolWidget(QWidget *w) {
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
    }
    if (w) {
        m_toolWidget = w;
        m_upLayout->addWidget(w);
    }
}

void ButtonPanel::clearToolWidget() {
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
    }
}

void ButtonPanel::pushWidget(QWidget *w) {
    m_layout->addWidget(w);
}


void ButtonPanel::initInternalPanel() {
    m_internalPanel = new QScrollArea(this);
    m_internalPanel->setAttribute(Qt::WA_NoMousePropagation);
    QWidget *widget = new QWidget();
    m_internalPanel->setWidget(widget);
    m_internalPanel->setWidgetResizable(true);

    m_layout = new QVBoxLayout();
    m_upLayout = new QVBoxLayout();
    m_layout->addLayout(m_upLayout);
    widget->setLayout(m_layout);

    QColor bgColor = palette().background().color();
    bgColor.setAlphaF(0.0);
    m_internalPanel->setStyleSheet(QStringLiteral("QScrollArea {background-color: %1}")
                                   .arg(bgColor.name()));

}
