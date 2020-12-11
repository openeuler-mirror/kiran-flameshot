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
#include "baseutils.h"
#include "textbutton.h"

#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/tools/toolbutton.h"
#include "src/tools/colorbutton.h"
#include "src/tools/rectbutton.h"
#include "src/tools/linebutton.h"

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
    const int TOOLBAR_HEIGHT = 50;
    const int TOOLBAR_WIDTH = 300;
    const int BUTTON_SPACING = 1;
    const int COLOR_NUM = 5;
    const QSize SAVEBTN_SIZE = QSize(33, 26);
}

//右键呼出的调色板
ColorPicker::ColorPicker(int m_thickness,QColor m_color, QWidget *parent) : QWidget(parent) {
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
    // extraSize represents the extra space needed for the highlight of the
    // selected color.
   /* const int extraSize = 6;
    double radius = (m_colorList.size()*m_colorAreaSize/1.3)/(3.141592);
    resize(radius*2 + m_colorAreaSize + extraSize,
           radius*2 + m_colorAreaSize+ extraSize);
    double degree = 360 / (m_colorList.size());
    double degreeAcum = degree;
    // this line is the radius of the circle which will be rotated to add
    // the color components.
    QLineF baseLine = QLineF(QPoint(radius+extraSize/2, radius+extraSize/2),
                             QPoint(radius*2, radius));

    for (int i = 0; i<m_colorList.size(); ++i) {
        m_colorAreaList.append(QRect(baseLine.x2(), baseLine.y2(),
                                 m_colorAreaSize, m_colorAreaSize));
        baseLine.setAngle(degreeAcum);
        degreeAcum += degree;
    }*/

    setStyleSheet(getFileContent(":/subtoolbar.qss"));
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setStyleSheet("QWidget{border-radius:4px;background-color:white;}");

    /*m_rectLabel = new QLabel(this);
    m_colorLabel = new QLabel(this);*/

    lastWidget = new QWidget();
    m_rectLabel = new QWidget(this);
    /*m_arrowLabel = new QWidget(this);
    m_lineLabel = new QWidget(this);*/
    m_textLabel = new QWidget(this);
    m_colorLabel = new QWidget(this);
   // m_thicknessLabel = new QWidget(this);

    m_rectLabel->setMinimumWidth(150);
    /*m_arrowLabel->setMinimumWidth(200);
    m_lineLabel->setMinimumWidth(200);*/
    m_textLabel->setMinimumWidth(150);

    /*m_rectLabel->setMinimumHeight(30);
    m_arrowLabel->setMinimumHeight(30);
    m_lineLabel->setMinimumHeight(30);
    m_textLabel->setMinimumHeight(30);*/

    m_rectLabel->setMouseTracking(true);
   /* m_arrowLabel->setMouseTracking(true);
    m_lineLabel->setMouseTracking(true);*/
    m_textLabel->setMouseTracking(true);
    m_colorLabel->setMouseTracking(true);


   /* m_rectLabel->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");
    m_arrowLabel->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");
    m_lineLabel->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");
    m_textLabel->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");
    m_colorLabel->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");*/

    //initThcikness(m_thickness);
    initRectLabel(m_thickness);
  /*  initArrowLabel(m_thickness);

    initLineLabel(m_thickness);*/
    initTextLabel();
    initColorLabel(m_color);

    QLabel* SeparatorLine = new QLabel();
    SeparatorLine->setFixedSize(1, 20);
    SeparatorLine->setObjectName("VerticalSeparatorLine");

    lastWidget = m_rectLabel;
    m_Layout = new QHBoxLayout();
    m_Layout->addWidget(m_rectLabel);
    /*m_Layout->addSpacing(8);
    m_Layout->addWidget(SeparatorLine);
    m_Layout->addSpacing(8);*/
    m_Layout->addWidget(m_colorLabel);
    m_Layout->addStretch();
    this->setLayout(m_Layout);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);

}

void ColorPicker::initRectLabel(int m_thickness)
{
    m_rectLabel->setStyleSheet(getFileContent(":/thicknessbutton.qss"));
    QButtonGroup* rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(true);
    QList<ToolButton*> btnList;
    //rectangle, oval...

    fineLine = new ToolButton(SMALL_SIZE);
    fineLine->setObjectName("FineLine");
    rectBtnGroup->addButton(fineLine);
    btnList.append(fineLine);
    mediumLine = new ToolButton(MEDIUM_SIZE);
    mediumLine->setObjectName("MediumLine");
    rectBtnGroup->addButton(mediumLine);
    btnList.append(mediumLine);
    thickLine = new ToolButton(LARGE_SIZE);
    thickLine->setObjectName("ThickLine");
    //thickLine->setStyleSheet("QPushButton{border-radius:15px;background-color:#000000;}");
    rectBtnGroup->addButton(thickLine);
    btnList.append(thickLine);
    //separator line...
    QLabel* vSeparatorLine = new QLabel();
    vSeparatorLine->setFixedSize(1, 20);
    vSeparatorLine->setObjectName("VerticalSeparatorLine");

    QHBoxLayout* rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(1);
    for (int i = 0; i < btnList.length(); i++) {
        //btnList[i]->setFixedSize(30,30);
       btnList[i]->setStyleSheet("border-radius:4px");
       rectLayout->addWidget(btnList[i]);
       if(m_thickness == btnList[i]->m_thickness)
           btnList[i]->setChecked(true);
       rectLayout->addSpacing(BUTTON_SPACING);
        connect(btnList[i], &ToolButton::updateThickness,
                this, &ColorPicker::updateThickness);
    }

    rectLayout->addStretch();
    m_rectLabel->setLayout(rectLayout);
    //addWidget(m_rectLabel);
    m_rectLabel->show();

}

/*
void ColorPicker::initArrowLabel(int m_thickness)
{
    //arrow
    QButtonGroup*  arrowBtnGroup = new QButtonGroup();
    arrowBtnGroup->setExclusive(true);
    QList<ToolButton*> arrowBtnList;
    ToolButton* arrowFineLine = new ToolButton(SMALL_SIZE);
    arrowFineLine->setObjectName("ArrowFine");
    arrowBtnGroup->addButton(arrowFineLine);
    arrowBtnList.append(arrowFineLine);
    ToolButton*  arrowMediumLine = new ToolButton(MEDIUM_SIZE);
    arrowMediumLine->setObjectName("ArrowMedium");
    arrowBtnGroup->addButton(arrowMediumLine);
    arrowBtnList.append(arrowMediumLine);
    ToolButton* arrowThickLine = new ToolButton(LARGE_SIZE);
    arrowThickLine->setObjectName("ArrowThick");
    arrowBtnGroup->addButton(arrowThickLine);
    arrowBtnList.append(arrowThickLine);
    //line
    QButtonGroup*  lineBtnGroup = new QButtonGroup();
    lineBtnGroup->setExclusive(true);
    //rectangle, oval...
    ToolButton* fineLine = new ToolButton(5);
    fineLine->setObjectName("ArrowFineLine");
    lineBtnGroup->addButton(fineLine);
    arrowBtnList.append(fineLine);
    ToolButton*  mediumLine = new ToolButton(5);
    mediumLine->setObjectName("ArrowMediumLine");
    lineBtnGroup->addButton(mediumLine);
    arrowBtnList.append(mediumLine);
    ToolButton* thickLine = new ToolButton(5);
    thickLine->setObjectName("ArrowThickLine");
    lineBtnGroup->addButton(thickLine);
    arrowBtnList.append(thickLine);
    //separator line...
    QLabel* vSeparatorLine = new QLabel();
    vSeparatorLine->setFixedSize(1, 20);
    vSeparatorLine->setObjectName("VerticalSeparatorLine");

    QButtonGroup* styleBtnGroup = new QButtonGroup;
    styleBtnGroup->setExclusive(true);
    LineButton*  lineBtn = new LineButton(1);
    lineBtn->setObjectName("LineBtn");
    lineBtn->setChecked(true);
    styleBtnGroup->addButton(lineBtn, 0);
    LineButton* arrowBtn = new LineButton(2);
    arrowBtn->setObjectName("ArrowBtn");
    //arrowBtn->setChecked(true);
    styleBtnGroup->addButton(arrowBtn, 1);
    connect(lineBtn, &LineButton::updateLine,
            this, &ColorPicker::updateLine);
    connect(arrowBtn, &LineButton::updateLine,
            this, &ColorPicker::updateLine);

    QHBoxLayout* arrowLayout = new QHBoxLayout();
    arrowLayout->setMargin(0);
    arrowLayout->setSpacing(0);
    arrowLayout->addSpacing(1);
    for (int j = 0; j < arrowBtnList.length(); j++) {
       arrowLayout->addWidget(arrowBtnList[j]);
       arrowLayout->addSpacing(BUTTON_SPACING);
       if(m_thickness == arrowBtnList[j]->m_thickness)
           arrowBtnList[j]->setChecked(true);
       arrowLayout->addSpacing(BUTTON_SPACING);
        connect(arrowBtnList[j], &ToolButton::updateThickness,
                this, &ColorPicker::updateThickness);
    }
    arrowLayout->addSpacing(5);
    arrowLayout->addWidget(vSeparatorLine);
    arrowLayout->addSpacing(8);
    arrowLayout->addWidget(lineBtn);
    arrowLayout->addSpacing(BUTTON_SPACING);
    arrowLayout->addWidget(arrowBtn);
    arrowLayout->addStretch();
    m_arrowLabel->setLayout(arrowLayout);

    fineLine->hide();
    mediumLine->hide();
    thickLine->hide();
}
*/

/*
void ColorPicker::initLineLabel(int m_thickness)
{
    //rectangle, oval...
    QButtonGroup* lineBtnGroup = new QButtonGroup();
    lineBtnGroup->setExclusive(true);
    QList<ToolButton*> btnList;

    ToolButton* fineLine = new ToolButton(SMALL_SIZE);
    fineLine->setObjectName("FineLine");
    lineBtnGroup->addButton(fineLine);
    btnList.append(fineLine);
    ToolButton*  mediumLine = new ToolButton(MEDIUM_SIZE);
    mediumLine->setObjectName("MediumLine");
    lineBtnGroup->addButton(mediumLine);
    btnList.append(mediumLine);
    ToolButton* thickLine = new ToolButton(LARGE_SIZE);
    thickLine->setObjectName("ThickLine");
    lineBtnGroup->addButton(thickLine);
    btnList.append(thickLine);

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addSpacing(1);
    for(int k = 0; k < btnList.length(); k++) {
       lineLayout->addWidget(btnList[k]);
       if(m_thickness == btnList[k]->m_thickness)
           btnList[k]->setChecked(true);
       lineLayout->addSpacing(BUTTON_SPACING);
       connect(btnList[k], &ToolButton::updateThickness,
               this, &ColorPicker::updateThickness);
    }

    lineLayout->addStretch();
    m_lineLabel->setLayout(lineLayout);
}*/

void ColorPicker::initTextLabel()
{
    m_textLabel->setStyleSheet(getFileContent(":/resources/qss/textbutton.qss"));

    textButton = new TextButton(SMALL_SIZE);
    textButton->setFixedSize(60,18);
    textButton->setStyleSheet("QComboBox{border-radius:4px;padding:2px 4px;background-color:#e9e9e9;}");
    connect(textButton, &TextButton::updateThickness,
            this, &ColorPicker::updateThickness);

     //int defaultFontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();

    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->addSpacing(2);
    textLayout->addWidget(textButton);
    textLayout->addSpacing(BUTTON_SPACING);

    textLayout->addStretch();
    m_textLabel->setLayout(textLayout);
}

void ColorPicker::initColorLabel(QColor m_color)
{
    m_colorLabel->setStyleSheet(getFileContent(":/colorbtn.qss"));
    //m_colorLabel->resize(400,50);

    QList<ColorButton*> colorBtnList;
    QButtonGroup* colorBtnGroup = new QButtonGroup(m_colorLabel);
    colorBtnGroup->setExclusive(true);

    for(int i = 0; i < COLOR_NUM; i ++) {
            ColorButton* colorBtn = new ColorButton(colorIndexOf(i));
            if(colorIndexOf(i) == m_color)
                colorBtn->setChecked(true);
            colorBtn->setObjectName(QString("ColorBtn%1").arg(i+1));
            colorBtn->setFixedSize(23,23);
            colorBtnList.append(colorBtn);
            colorBtnGroup->addButton(colorBtn);
        }

    QHBoxLayout* colorLayout = new QHBoxLayout();
    colorLayout->setMargin(0);
    colorLayout->setSpacing(5);
    colorLayout->addSpacing(10);
    for(int i = 0; i < colorBtnList.length(); i++) {
        colorLayout->addWidget(colorBtnList[i]);
        colorLayout->addSpacing(10);
        connect(colorBtnList[i], &ColorButton::updatePaintColor,
                this, &ColorPicker::updateColor);
    }
    colorLayout->addSpacing(10);
    colorLayout->addStretch();
    m_colorLabel->setLayout(colorLayout);

    //addWidget(m_colorLabel);
    //m_colorLabel->hide();

    //addWidget(m_colorLabel);
}

void ColorPicker::updateLine(int m_line)
{
    emit lineSelected(m_line);
}

void ColorPicker::updateStyle(int m_style)
{
    emit styleSelected(m_style);
}

void ColorPicker::updateThickness(int thickness)
{
   printf("#######\n");
   emit thicknessSelected(thickness);
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
    printf("test:%d\n",thickness);
    if(thickness == SMALL_SIZE)
    {
        fineLine->setChecked(true);
        textButton->setCurrentIndex(0);
    }
    else if(thickness == MEDIUM_SIZE)
    {
        mediumLine->setChecked(true);
        textButton->setCurrentIndex(1);
    }
    else if(thickness == LARGE_SIZE)
    {
        thickLine->setChecked(true);
        textButton->setCurrentIndex(2);
    }
    else
    {
        fineLine->setChecked(false);
        mediumLine->setChecked(false);
        thickLine->setChecked(false);
    }
}

void ColorPicker::show(QString toolName) {
    //grabMouse();
    int toolNum = 99;

    if(toolName == "铅笔" || toolName == "直线" || toolName == "标记")
        //toolNum = 1;
        toolNum = 3;
    else if(toolName == "箭头")
        //toolNum = 3;
        toolNum = 2;
    else if(toolName == "矩形选择" || toolName == "实心矩形" || toolName == "圆环")
        toolNum = 3;
    else if(toolName == "文本")
        toolNum = 4;

    if(!m_rectLabel->isHidden())
        lastWidget = m_rectLabel;
    /*else if(!m_lineLabel->isHidden())
        lastWidget = m_lineLabel;
    else if(!m_arrowLabel->isHidden())
        lastWidget = m_arrowLabel;*/
    else if(!m_textLabel->isHidden())
        lastWidget = m_textLabel;

    switch(toolNum) {
   /* case 1:
        if(lastWidget != m_lineLabel)
            m_Layout->replaceWidget(lastWidget,m_lineLabel);
        m_textLabel->hide();
        m_rectLabel->hide();
        m_arrowLabel->hide();
        m_lineLabel->show();
        m_colorLabel->show();
        break;*/
    /*case 2:
        if(lastWidget != m_arrowLabel)
            m_Layout->replaceWidget(lastWidget,m_arrowLabel);
        m_textLabel->hide();
        m_rectLabel->hide();
        m_arrowLabel->show();
        m_lineLabel->hide();
        m_colorLabel->show();
        break;*/
    case 1:
    case 2:
    case 3:
        if(lastWidget != m_rectLabel)
            m_Layout->replaceWidget(lastWidget,m_rectLabel);
        m_textLabel->hide();
        m_rectLabel->show();
        /*m_arrowLabel->hide();
        m_lineLabel->hide();*/
        m_colorLabel->show();
        break;
    case 4:
        if(lastWidget != m_textLabel)
            m_Layout->replaceWidget(lastWidget,m_textLabel);
        m_textLabel->show();
        m_rectLabel->hide();
       /* m_arrowLabel->hide();
        m_lineLabel->hide();*/
        m_colorLabel->show();
        break;
    default:
        m_textLabel->hide();
        m_rectLabel->hide();
        /*m_arrowLabel->hide();
        m_lineLabel->hide();*/
        m_colorLabel->hide();
        break;
    }
    QWidget::show();
}

void ColorPicker::hide() {
    QWidget::hide();
}

void ColorPicker::paintEvent(QPaintEvent *) {
    /*QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QVector<QRect> rects = handleMask();
    painter.setPen(QColor(Qt::black));
    for (int i = 0; i < rects.size(); ++i) {
        // draw the highlight when we have to draw the selected color
        if (m_drawColor == QColor(m_colorList.at(i))) {
            QColor c = QColor(m_uiColor);
            c.setAlpha(155);
            painter.setBrush(c);
            c.setAlpha(100);
            painter.setPen(c);
            QRect highlight = rects.at(i);
            highlight.moveTo(highlight.x() - 3, highlight.y() - 3);
            highlight.setHeight(highlight.height() + 6);
            highlight.setWidth(highlight.width() + 6);
            //painter.drawRect(highlight);
            //painter.drawRoundRect(highlight, 100, 100);
            painter.setPen(QColor(Qt::white));
        }
        painter.setBrush(QColor(m_colorList.at(i)));
        //painter.drawRoundRect(rects.at(i), 100, 100);
        painter.drawRect(rects.at(i));
    }*/
}

void ColorPicker::mouseMoveEvent(QMouseEvent *e) {
    /*for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_drawColor = m_colorList.at(i);
            emit colorSelected(m_drawColor);
            update();
            break;
        }
    }*/
}

/*QVector<QRect> ColorPicker::handleMask() const {
    QVector<QRect> areas;
    for (const QRect &rect: m_colorAreaList) {
        areas.append(rect);
    }
    return areas;
}*/
