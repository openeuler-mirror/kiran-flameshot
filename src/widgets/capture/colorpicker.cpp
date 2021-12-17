/// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Kiran-Flameshot.
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

#include "colorpicker.h"
#include "src/tools/text/textbutton.h"

#include "src/utils/baseutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/tools/toolbutton.h"
#include "src/tools/colorbutton.h"
#include "src/tools/rectbutton.h"
#include "src/tools/linebutton.h"
#include "src/widgets/capture/toowidget.h"
#include "src/widgets/capture/thicknesswidget.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <QComboBox>

namespace {
    const int TOOLBAR_HEIGHT = 30;
    const int TOOLBAR_WIDTH = 300;
    const int BUTTON_SPACING = 1;
    const int COLOR_NUM = 5;
    const QSize SAVEBTN_SIZE = QSize(33, 26);
}

//右键呼出的调色板
ColorPicker::ColorPicker(int thickness,QColor m_color, QWidget *parent) : QWidget(parent) {
    //this->resize(50,300);
    setCursor(Qt::ArrowCursor);
    this->setMouseTracking(true);
    /*this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);*/

    ConfigHandler config;
    m_colorList = config.getUserColors();
    m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
    // save the color values in member variables for faster access
    m_uiColor = config.uiMainColorValue();
    m_drawColor = config.drawColorValue();
    m_thickness = thickness;

    setStyleSheet(getFileContent(":/qss/subtoolbar.qss"));
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setStyleSheet("QWidget{border-radius:4px;background-color:white;}");

    label = new QLabel(this);
    label->setFixedSize(222,30);
    QPalette pal(label->palette());
    pal.setColor(QPalette::Background, Qt::white);
    label->setAutoFillBackground(true);

    tWidget = new Toowidget(m_drawColor,this);
    connect(tWidget, &Toowidget::colorSelected, this, &ColorPicker::updateColor);
    tWidget->setFixedSize(222,30);
    tWidget->show();

    thicknessWidget  = new ThicknessWidget(m_thickness,this);
    connect(thicknessWidget, &ThicknessWidget::thicknessSelected, this, &ColorPicker::updateThickness_tool);
    connect(this,&ColorPicker::thicknessCh_tool, thicknessWidget, &ThicknessWidget::updateThickness);
    thicknessWidget->setFixedSize(66,30);
    //thicknessWidget->setStyleSheet(getFileContent(":/resources/qss/textbutton.qss"));
    thicknessWidget->hide();

    m_textLabel = new QLabel();
    m_textLabel->resize(100,30);

    textButton = new TextButton(m_thickness);
    textButton->setFixedSize(44,18);
    textButton->setStyleSheet("QComboBox{border-radius:3px;padding:2px 4px;background-color:#e9e9e9;color: black}");
    connect(textButton, &TextButton::updateThickness,
            this, &ColorPicker::updateThickness_font);
    //textButton->setStyleSheet(getFileContent(":/textbutton.qss"));

    QHBoxLayout* rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(8);
    rectLayout->addWidget(textButton);
    //rectLayout->addWidget(m_textLabel);

    rectLayout->addSpacing(8);
    thicknessWidget->setLayout(rectLayout);

    update();
}

void ColorPicker::updateLine(int m_line)
{
    emit lineSelected(m_line);
}

void ColorPicker::updateStyle(int m_style)
{
    emit styleSelected(m_style);
}

void ColorPicker::updateThickness_tool(int thickness)
{
    m_thickness = thickness;
    emit thicknessSelected(m_thickness);
//    emit thicknessCh_font(m_thickness);
    thicknessCh_font(m_thickness);
}

void ColorPicker::updateThickness_font(int thickness)
{
    m_thickness = thickness;
    emit thicknessSelected(m_thickness);
    emit thicknessCh_tool(m_thickness);
}

void ColorPicker::updateColor(QColor color)
{
    m_drawColor = color;
    emit colorSelected(m_drawColor);
    update();
}

QColor ColorPicker::drawColor() {
    return m_drawColor;
}

void ColorPicker::thicknessChanged(int thickness)
{
    m_thickness = thickness;
    emit thicknessCh_tool(m_thickness);
    thicknessCh_font(m_thickness);
}

void ColorPicker::show(QString toolName) {
    if(toolName == "文本")
    {
        thicknessWidget->hide();
        textButton->show();
    }
    else
    {
        thicknessWidget->show();
        textButton->hide();
    }
    QWidget::show();
}

void ColorPicker::hide() {
    QWidget::hide();
}

void ColorPicker::thicknessCh_font(int thickness)
{
    if(m_thickness == SMALL_SIZE)
    {
        textButton->setCurrentIndex(0);
    }
    else if(m_thickness == MEDIUM_SIZE)
    {
        textButton->setCurrentIndex(1);
    }
    else if(m_thickness == LARGE_SIZE)
    {
        textButton->setCurrentIndex(2);
    }
}
