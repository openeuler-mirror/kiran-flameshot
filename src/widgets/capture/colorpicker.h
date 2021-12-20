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
#include <QStackedWidget>
#include <QHBoxLayout>
#include "src/tools/rectbutton.h"
#include "src/tools/toolbutton.h"
#include "src/tools/text/textbutton.h"
#include "toowidget.h"
#include "thicknesswidget.h"

class ColorPicker : public QWidget {
    Q_OBJECT
public:
    explicit ColorPicker(int thickness, QColor m_color, QWidget *parent = nullptr);

    QColor drawColor();

    void show(QString toolName);
    void hide();
    void initColorLabel(QColor m_color);
    void initThcikness(int m_thickness);
    void initRectLabel(int m_thickness);
    void initArrowLabel(int m_thickness);
    void initLineLabel(int m_thickness);
    void initTextLabel();

public slots:
    void updateColor(QColor color);
    void updateThickness_tool(int thickness);
    void updateThickness_font(int thickness);
    void updateStyle(int m_style);
    void updateLine(int m_line);
    void thicknessChanged(int thickness);

signals:
    void colorSelected(QColor c);
    void thicknessSelected(int thickness);
    void styleSelected(int m_style);
    void lineSelected(int m_line);
    void thicknessCh_tool(int thickness);


protected:
    void thicknessCh_font(int thickness);

private:
    int m_colorAreaSize;
    QVector<QRect> m_colorAreaList;
    QVector<QColor> m_colorList;

    QColor m_uiColor, m_drawColor;
    int m_thickness;

    QWidget* m_rectLabel;
    QWidget* m_colorLabel;
    QWidget* m_thicknessLabel;

    //QLabel* m_rectLabel;
//    QWidget* m_arrowLabel;
//    QWidget* m_lineLabel;
   //QWidget* m_textLabel;
    //QLabel* m_colorLabel;
    QWidget* m_saveLabel;

    QWidget *lastWidget;
    QHBoxLayout* m_Layout;

    RectButton* blurBtn;
    RectButton* mosaicBtn;

    ToolButton* fineLine;
    ToolButton* mediumLine;
    ToolButton* thickLine;
    TextButton* textButton;

    QLabel *label;
    QLabel *m_textLabel;

    Toowidget *tWidget;
    ThicknessWidget *thicknessWidget;
};
