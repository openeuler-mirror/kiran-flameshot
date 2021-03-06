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
#include <QLabel>
#include <QHBoxLayout>

class Toowidget : public QWidget
{
    Q_OBJECT
public:
    explicit Toowidget(QColor m_color, QWidget *parent = nullptr);

public slots:
    void updateColor(QColor c);

signals:
    void colorSelected(QColor c);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    QVector<QRect> handleMask() const;

private:
    int m_colorAreaSize;

    QVector<QRect> m_colorAreaList;
    QVector<QColor> m_colorList;

    QColor m_uiColor, m_drawColor, m_selectColor;

    bool m_inselection = false;

};


