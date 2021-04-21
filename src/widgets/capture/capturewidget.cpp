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

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>


#include "src/widgets/panel/buttonpanel.h"
#include "zoomIndicator.h"
#include "baseutils.h"

#include <QDir>
#include "capturewidget.h"
#include "src/widgets/capture/hovereventfilter.h"
#include "src/widgets/panel/sidepanelwidget.h"
#include "src/utils/colorutils.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/capture/notifierbox.h"
#include "src/widgets/capture/colorpicker.h"
#include "src/utils/screengrabber.h"
#include "src/utils/systemnotification.h"
#include "src/utils/screenshotsaver.h"
#include "src/core/controller.h"
#include "src/widgets/capture/modificationcommand.h"
#include <QUndoView>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QShortcut>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QBuffer>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QDebug>

//#include <window-manager.h>

#include <X11/Xlib.h>

namespace {
const int RECORD_MIN_SIZE = 10;
const int SPACING = 10;
const int TOOLBAR_Y_SPACING = 8;
const int CURSOR_WIDTH = 8;
const int CURSOR_HEIGHT = 18;
const int INDICATOR_WIDTH =  99;
const qreal RESIZEPOINT_WIDTH = 15;
const int BAR_WIDTH = 530;
const int BAR_HEIGHT = 30;
}

// CaptureWidget is the main component used to capture the screen. It contains an
// are of selection with its respective buttons.

// enableSaveWIndow
CaptureWidget::CaptureWidget(const uint id, const QString &savePath,
                             bool fullScreen, QWidget *parent) :
    QWidget(parent), m_mouseIsClicked(false), m_rightClick(false),
    m_newSelection(false), m_grabbing(false), m_captureDone(false),
    m_previewEnabled(true), m_adjustmentButtonPressed(false), m_activeButton(nullptr),
    m_activeTool(nullptr), m_toolWidget(nullptr),
    m_mouseOverHandle(SelectionWidget::NO_SIDE), m_id(id)
{
    // Base config of the widget
    m_eventFilter = new HoverEventFilter(this);
    connect(m_eventFilter, &HoverEventFilter::hoverIn,
            this, &CaptureWidget::childEnter);
    connect(m_eventFilter, &HoverEventFilter::hoverOut,
            this, &CaptureWidget::childLeave);
    setAttribute(Qt::WA_DeleteOnClose);
    m_showInitialMsg = m_config.showHelpValue();
    m_opacity = m_config.contrastOpacityValue();

    m_bar = new QWidget(this);
    m_bar->resize(BAR_WIDTH,BAR_HEIGHT);
    m_bar->setMouseTracking(true);
    m_bar->setStyleSheet("QWidget{border-radius:4px;padding:2px 4px;background-color:white;}");
    m_bar->move(-500,-500);
    m_bar->hide();

    setMouseTracking(true);
    initContext(savePath, fullScreen);
    initShortcuts();


#ifdef Q_OS_WIN
    // Top left of the whole set of screens
    QPoint topLeft(0,0);
#endif
    if (fullScreen) {
        // Grab Screenshot
        bool ok = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(ok);
        if(!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            this->close();
        }
        m_context.origScreenshot = m_context.screenshot;

#ifdef Q_OS_WIN
        setWindowFlags(Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Popup);

        for (QScreen *const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();
            if (topLeft.x() > topLeftScreen.x() ||
                    topLeft.y() > topLeftScreen.y()) {
                topLeft = topLeftScreen;
            }
        }
        move(topLeft);
#else
        setWindowFlags(Qt::BypassWindowManagerHint
                       | Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Tool);
#endif
        resize(pixmap().size());
    }
    //Create toptips
    m_toptips = new TopTips(this);
    m_toptips->hide();

    // Create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    QVector<QRect> areas;
    if (m_context.fullscreen) {
        for (QScreen *const screen : QGuiApplication::screens()) {
            QRect r = screen->geometry();
#ifdef Q_OS_WIN
            r.moveTo(r.topLeft() - topLeft);
#endif
            areas.append(r);
        }
    } else {
        areas.append(rect());
    }
    m_buttonHandler->updateScreenRegions(areas);
    m_buttonHandler->hide();

    initSelection();
    updateCursor();

    initOriginUI();
    initSecondUI();

    m_formtip = new FormTip(this);
    makeChild(m_formtip);
    m_formtip->move(-500,-500);
    m_formtip->hide();

    // Init color picker
    m_colorPicker = new ColorPicker(m_context.thickness,m_context.color,this);
    connect(m_colorPicker, &ColorPicker::colorSelected,
            this, &CaptureWidget::setDrawColor);
    connect(m_colorPicker, &ColorPicker::thicknessSelected,
            this, &CaptureWidget::setDrawThickness);
//    connect(m_colorPicker, &ColorPicker::styleSelected,
//            this, &CaptureWidget::setDrawRectStyle);
//    connect(m_colorPicker, &ColorPicker::lineSelected,
//            this, &CaptureWidget::setDrawLineStyle);
    connect(this,&CaptureWidget::thicknessCh,
            m_colorPicker, &ColorPicker::thicknessChanged);
    /*QLabel* m_hSeparatorLine = new QLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_colorPicker, 0, Qt::AlignVCenter);
    setLayout(vLayout);*/
    makeChild(m_colorPicker);
    m_colorPicker->move(-500,-500);
    m_colorPicker->hide();

    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();

    connect(&m_undoStack, &QUndoStack::indexChanged,
            this, [this](int){ this->update(); });
    initPanel();
}

CaptureWidget::~CaptureWidget() {
    if (m_captureDone) {
        emit captureTaken(m_id, this->pixmap());
    } else {
        emit captureFailed(m_id);
    }
    m_config.setdrawThickness(m_context.thickness);
}

void CaptureWidget::initOriginUI()
{
    QPoint curPos = this->cursor().pos();

   const qreal ratio = devicePixelRatioF();
   //m_swUtil = DScreenWindowsUtil::instance(curPos);
   //m_screenNum =  m_swUtil->getScreenNum();
   //m_backgroundRect = m_swUtil->backgroundRect();

   m_backgroundRect =  m_selection->getArea();
  /* m_backgroundRect.setWidth(INDICATOR_WIDTH);
   m_backgroundRect.setHeight(INDICATOR_WIDTH);*/
/*
   for(QScreen *const screen : QGuiApplication::screens()){
       QRect r = screen->geometry();
       m_backgroundRect = r;
   }*/

   m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());

   move(m_backgroundRect.topLeft() * ratio);
   //this->setFixedSize(m_backgroundRect.size());
   initBackground();

    m_zoomIndicator = new ZoomIndicator(this);
    m_zoomIndicator->hide();

    m_isFirstReleaseButton = false;
/*
    m_isFirstDrag = false;
    m_isFirstMove = false;
    m_isFirstPressButton = false;
    m_isFirstReleaseButton = false;

    m_recordX = 0;
    m_recordY = 0;
    m_recordWidth = 0;
    m_recordHeight = 0;

    qreal ration =  this->devicePixelRatioF();
    QIcon icon(":/resize_handle_big.svg");
    m_resizeBigPix = icon.pixmap(QSize(RESIZEPOINT_WIDTH,RESIZEPOINT_WIDTH));
    m_resizeBigPix.setDevicePixelRatio(ration);

    m_dragRecordX = -1;
    m_dragRecordY = -1;

    m_needDrawSelectedPoint = false;
    m_mouseStatus = ShotMouseStatus::Shoting;

    m_selectAreaName = "";

    m_isShapesWidgetExist = false;*/
}


void CaptureWidget::initSecondUI()
{
    //m_menuController = new MenuController(this);
        return;
    /*m_toolBar = new ToolBar(this);
    m_toolBar->hide();*/
}

QPixmap CaptureWidget::getPixmapofRect(const QRect &rect)
{
    QRect r(rect.topLeft() * devicePixelRatioF(), rect.size());

    QList<QScreen*> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        if ((*it)->geometry().contains(r)) {
            return (*it)->grabWindow(this->winId(), rect.x(), rect.y(), rect.width(), rect.height());
        }
    }

    return QPixmap();
}

void CaptureWidget::initBackground()
{
    m_backgroundPixmap = getPixmapofRect(m_backgroundRect);
    m_resultPixmap = m_backgroundPixmap;
    //TempFile::instance()->setFullScreenPixmap(m_backgroundPixmap);
}

// redefineButtons retrieves the buttons configured to be shown with the
// selection in the capture
void CaptureWidget::updateButtons() {
    m_uiColor = m_config.uiMainColorValue();
    m_contrastUiColor = m_config.uiContrastColorValue();

    auto buttons = m_config.getButtons();
    QVector<CaptureButton*> vectorButtons;
    /*QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setMargin(0);
    vlayout->setSpacing(0);*/

    QHBoxLayout* rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(9);

    for (const CaptureButton::ButtonType &t: buttons) {
        CaptureButton *b = new CaptureButton(t, this);

        if (t == CaptureButton::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        else if(t == CaptureButton::TYPE_SAVEOPTION)
        {

            QLabel* SeparatorLine = new QLabel();
            SeparatorLine->setFixedSize(1, 20);
            SeparatorLine->setStyleSheet("QLabel{border: 1px solid rgb(199,199,199);}");

            rectLayout->addSpacing(9);
            rectLayout->addWidget(SeparatorLine);

            m_menuController =  new MenuController(this);
            QMenu* m_menu = m_menuController->getMenu();

            b->setMenu(m_menu);

            m_menuController->setDefaultCheck(m_context.savetip);
            m_menuController->setDefaultFormat(m_context.picture_format);

            connect(m_menuController, &MenuController::setPformat, this, &CaptureWidget::setPformat);
            connect(m_menuController, &MenuController::setSaveTip, this, &CaptureWidget::setSaveTip);
            connect(m_menuController, &MenuController::menuHide, this, &CaptureWidget::menuHide);
        }
        //vlayout->addWidget(b);
        rectLayout->addSpacing(9);
        rectLayout->addWidget(b);


        b->setColor(m_uiColor);
        makeChild(b);



        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        vectorButtons << b;
    }

    rectLayout->addSpacing(0);
    rectLayout->addStretch();

    m_bar->setLayout(rectLayout);
    m_bar->hide();
    makeChild(m_bar);

    m_buttonHandler->setButtons(vectorButtons);

}

QPixmap CaptureWidget::pixmap() {
    QPixmap p;
    if (m_toolWidget && m_activeTool) {
        p = m_context.selectedScreenshotArea().copy();
        QPainter painter(&p);
        m_activeTool->process(painter, p);
    } else {
        p = m_context.selectedScreenshotArea();
    }
    return m_context.selectedScreenshotArea();
}

void CaptureWidget::deleteToolwidgetOrClose() {
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
        m_toolWidget = nullptr;
    } else {
        close();
    }
}

void CaptureWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_context.screenshot);
    if(m_flag)
    {
        //painter.fillPath(path,QBrush(Qt::black));
        m_flag = false;
    }
    if (m_activeTool && m_mouseIsClicked) {
        painter.save();
        m_activeTool->process(painter, m_context.screenshot);
        painter.restore();
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview() &&
               m_previewEnabled && m_inselection)
    {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }

    QColor overlayColor(0, 0, 0, m_opacity);
    painter.setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized().adjusted(0, 0, -1, -1);
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter.setClipRegion(grey);
    painter.drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter.setClipRect(rect());

    if (m_showInitialMsg) {
        QRect helpRect = QGuiApplication::primaryScreen()->geometry();
        helpRect.moveTo(mapFromGlobal(helpRect.topLeft()));

        QString helpTxt = tr("Press Enter to capture the screen."
                             "\nPress Right Click to show the color picker."
                             "\nUse the Mouse Wheel to change the thickness of your tool."
                             "\nUse ctrl+c to save the picture to the clipboard."
                             "\nUse ctrl+s to save the picture to the set location.");

        // We draw the white contrasting background for the text, using the
        //same text and options to get the boundingRect that the text will have.
        QRectF bRect = painter.boundingRect(helpRect, Qt::AlignCenter, helpTxt);

        // These four calls provide padding for the rect
        const int margin = QApplication::fontMetrics().height() / 2;
        bRect.setWidth(bRect.width() + margin);
        bRect.setHeight(bRect.height() + margin);
        bRect.setX(bRect.x() - margin);
        bRect.setY(bRect.y() - margin);

        QColor rectColor(m_uiColor);
        rectColor.setAlpha(180);
        QColor textColor((ColorUtils::colorIsDark(rectColor) ?
                              Qt::white : Qt::black));

        painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
        painter.setPen(QPen(textColor));

        //绘制提示框和对应文本
        painter.drawRect(bRect);
        painter.drawText(helpRect, Qt::AlignCenter, helpTxt);
    }

    if (m_selection->isVisible()) {
        // paint handlers
        painter.setPen(m_uiColor);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(m_uiColor);
        for(auto r: m_selection->handlerAreas()) {
            painter.drawRoundRect(r, 100, 100);
        }
    }
}

void CaptureWidget::mousePressEvent(QMouseEvent *e) {

    if (!m_isFirstReleaseButton) {
                m_isFirstReleaseButton = true;
    }

    if(m_selection->geometry().contains(this->mapFromGlobal(QCursor::pos())))
        m_inselection = true;
    else
        m_inselection = false;

    if (e->button() == Qt::RightButton) {
        m_rightClick = true;
        /*if(m_selection->x() + m_selection->width() > m_context.desktop->screenGeometry().width() - 60)
            m_colorPicker->move( m_selection->x() + m_selection->width() - 10 - m_colorPicker->width(),
                                 m_selection->y() + m_selection->height()/2 - m_colorPicker->height()/2);
        else
            m_colorPicker->move( m_selection->x() + m_selection->width() + 10,
                                 m_selection->y() + m_selection->height()/2 - m_colorPicker->height()/2);
        m_colorPicker->show();*/

    } else if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;
        // Click using a tool
        if (m_activeButton && m_inselection) {
            if (m_activeTool) {
                if (m_activeTool->isValid() && m_toolWidget) {
                    pushToolToStack();
                } else {
                    m_activeTool->deleteLater();
                }
                if (m_toolWidget) {
                    m_toolWidget->deleteLater();
                    return;
                }
            }

            m_activeTool = m_activeButton->tool()->copy(this);


            connect(this, &CaptureWidget::colorChanged,
                    m_activeTool, &CaptureTool::colorChanged);
            connect(this, &CaptureWidget::thicknessChanged,
                    m_activeTool, &CaptureTool::thicknessChanged);
            connect(m_activeTool, &CaptureTool::requestAction,
                    this, &CaptureWidget::handleButtonSignal);

            m_activeTool->drawStart(m_context);
            return;
        }
        else if(m_activeButton)
            return;

        if(m_bar->geometry().contains(this->mapFromGlobal(QCursor::pos())))
            return ;

        m_dragStartPoint = e->pos();
        m_selection->saveGeometry();
        // New selection
        if (!m_selection->geometry().contains(e->pos()) &&
                m_mouseOverHandle == SelectionWidget::NO_SIDE)
        {
            m_selection->setGeometry(QRect(e->pos(), e->pos()));
            m_selection->setVisible(false);
            m_newSelection = true;
            m_buttonHandler->hide();
            m_bar->hide();
            update();
        } else {
            m_grabbing = true;
        }
    }
    updateCursor();
}


void CaptureWidget::mouseMoveEvent(QMouseEvent *e) {
    m_context.mousePos = e->pos();

    if(m_selection->geometry().contains(this->mapFromGlobal(QCursor::pos())))
        m_inselection = true;
    else
        m_inselection = false;

    if(QGuiApplication::keyboardModifiers() == Qt::ALT)
    {
        m_zoomIndicator->hide();
        /*for(int i = 0; i < test.length();i++)
        {
            qDebug() << test;
        }
*/
        /*m_selection->setVisible(true);
        m_selection->setGeometry(
                QRect(m_dragStartPoint, m_context.mousePos).normalized());*/
    }

    //if (m_selection->width() > 0 && m_selection->height >0 && !m_needSaveScreenshot && this->isVisible()) {
    if (m_selection->width() > 0 && m_selection->height() >0 && this->isVisible()) {
        m_toptips->updateTips(QPoint(m_selection->x(), m_selection->y()),
                                                     QString("%1X%2").arg(m_selection->width()).arg(m_selection->height()));
        m_zoomIndicator->hide();
    }

   // if(!m_buttonHandler->isVisible()&&m_isFirstReleaseButton)
    if(!m_buttonHandler->isVisible())
    {
        QPoint curPos = this->cursor().pos();
        QPoint tmpPos;
        //QRect m_backgroundRect = m_selection->geometry();
        QPoint topLeft = m_backgroundRect.topLeft() * devicePixelRatioF();

        if(curPos.x() + INDICATOR_WIDTH > m_context.desktop->screenGeometry().width() - 120)
        {
            tmpPos.setX(curPos.x() - INDICATOR_WIDTH);
        }
        else if (curPos.x() + INDICATOR_WIDTH + CURSOR_WIDTH > topLeft.x()
               + m_backgroundRect.width()) {
             tmpPos.setX(curPos.x() + 1);
        }
        else {
           tmpPos.setX(curPos.x() + CURSOR_WIDTH);
        }

        if(curPos.y() + INDICATOR_WIDTH > m_context.desktop->screenGeometry().height() - 120)
        {
            tmpPos.setY(curPos.y() - INDICATOR_WIDTH);
        }
        else if (curPos.y() + INDICATOR_WIDTH > topLeft.y() + m_backgroundRect.height()) {
            tmpPos.setY(curPos.y() + 1);
         }
        else{
           tmpPos.setY(curPos.y() + CURSOR_HEIGHT);
        }

        m_zoomIndicator->showMagnifier(QPoint(
           std::max(tmpPos.x() - topLeft.x(), 0),
           std::max(tmpPos.y() - topLeft.y(), 0)));
    }


    if (m_mouseIsClicked && !m_activeButton) {
        if (m_buttonHandler->isVisible()) {
            m_buttonHandler->hide();
            m_bar->hide();
        }
        if (m_newSelection) {
            m_selection->setVisible(true);
            m_selection->setGeometry(
                    QRect(m_dragStartPoint, m_context.mousePos).normalized());
            update();
        } else if (m_mouseOverHandle == SelectionWidget::NO_SIDE) {
            // Moving the whole selection
            QRect initialRect = m_selection->savedGeometry().normalized();
            QPoint newTopLeft = initialRect.topLeft() + (e->pos() - m_dragStartPoint);
            QRect finalRect(newTopLeft, initialRect.size());

            if (finalRect.left() < rect().left()) {
                finalRect.setLeft(rect().left());
            } else if (finalRect.right() > rect().right()) {
                finalRect.setRight(rect().right());
            }
            if (finalRect.top() < rect().top()) {
                finalRect.setTop(rect().top());
            } else if (finalRect.bottom() > rect().bottom()) {
                finalRect.setBottom(rect().bottom());
            }
            m_selection->setGeometry(finalRect.normalized().intersected(rect()));
            update();
        } else {
            cursor().pos();
            // Dragging a handle
            QRect r = m_selection->savedGeometry();
            QPoint offset = e->pos() - m_dragStartPoint;
            bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

            using sw = SelectionWidget;
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE
                    || m_mouseOverHandle == sw::TOP_SIDE
                    || m_mouseOverHandle == sw::TOPRIGHT_SIDE)
            { // dragging one of the top handles
                r.setTop(r.top() + offset.y());
                if (symmetryMod) {
                    r.setBottom(r.bottom() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE
                    || m_mouseOverHandle == sw::LEFT_SIDE
                    || m_mouseOverHandle == sw::BOTTONLEFT_SIDE)
            { // dragging one of the left handles
                r.setLeft(r.left() + offset.x());
                if (symmetryMod) {
                    r.setRight(r.right() - offset.x());
                }
            }
            if (m_mouseOverHandle == sw::BOTTONLEFT_SIDE
                    || m_mouseOverHandle == sw::BOTTON_SIDE
                    || m_mouseOverHandle == sw::BOTTONRIGHT_SIDE)
            { // dragging one of the bottom handles
                r.setBottom(r.bottom() + offset.y());
                if (symmetryMod) {
                    r.setTop(r.top() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPRIGHT_SIDE
                    || m_mouseOverHandle == sw::RIGHT_SIDE
                    || m_mouseOverHandle == sw::BOTTONRIGHT_SIDE)
            { // dragging one of the right handles
                r.setRight(r.right() + offset.x());
                if (symmetryMod) {
                    r.setLeft(r.left() - offset.x());
                }
            }
            m_selection->setGeometry(r.intersected(rect()).normalized());
            update();
        }
    } else if (m_mouseIsClicked && m_activeTool && m_inselection) {

        if(e->pos().x() < m_selection->x() && e->pos().y() < m_selection->y()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x(),m_selection->y()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x(),m_selection->y()));
            }
        }
        else if(e->pos().x() < m_selection->x() && e->pos().y() > m_selection->y() + m_selection->height()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x(),m_selection->y() + m_selection->height()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x(),m_selection->y() + m_selection->height()));
            }
        }
        else if(e->pos().x() > m_selection->x() + m_selection->width() && e->pos().y() < m_selection->y()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x() + m_selection->width(),m_selection->y()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x() + m_selection->width(),m_selection->y()));
            }
        }

        else if(e->pos().x() > m_selection->x() + m_selection->width() &&
                    e->pos().y() > m_selection->y() + m_selection->height()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x() + m_selection->width(),m_selection->y() + m_selection->height()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x() + m_selection->width(),m_selection->y() + m_selection->height()));
            }
        }

        else if(e->pos().x() > m_selection->x() + m_selection->width()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x() + m_selection->width(),e->pos().y()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x() + m_selection->width(),e->pos().y()));
            }
        }

        else if(e->pos().x() < m_selection->x()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(m_selection->x(),e->pos().y()));
            } else {
                m_activeTool->drawMove(QPoint(m_selection->x(),e->pos().y()));
            }
        }

        else if(e->pos().y() > m_selection->y() + m_selection->height()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(e->pos().x(),m_selection->y() + m_selection->height()));
            } else {
                m_activeTool->drawMove(QPoint(e->pos().x(),m_selection->y() + m_selection->height()));
            }
        }

        else if(e->pos().y() < m_selection->y()){
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(QPoint(e->pos().x(),m_selection->y()));
            } else {
                m_activeTool->drawMove(QPoint(e->pos().x(),m_selection->y()));
            }
        }
        else
        {
        // drawing with a tool
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(e->pos());
            } else {
                m_activeTool->drawMove(e->pos());
            }
        }
        update();
        // Hides the buttons under the mouse. If the mouse leaves, it shows them.
        if (m_buttonHandler->buttonsAreInside()) {
            const bool containsMouse = m_buttonHandler->contains(m_context.mousePos);
            if (containsMouse) {
                //m_buttonHandler->hide();
                //m_bar->hide();
            } else {
                //m_buttonHandler->show();
                //m_bar->show();
            }
        }
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    } else {
        if (!m_selection->isVisible()) {
            return;
        }
        m_mouseOverHandle = m_selection->getMouseSide(m_context.mousePos);
        updateCursor();
    }
}

void CaptureWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_colorPicker->hide();
        m_formtip->hide();
        m_rightClick = false;
    // when we end the drawing we have to register the last  point and
    //add the temp modification to the list of modifications
    } else if (m_mouseIsClicked && m_activeTool) {

        m_activeTool->drawEnd(m_context.mousePos);
        if (m_activeTool->isValid()) {
            pushToolToStack();
        } else if (!m_toolWidget){
            m_activeTool->deleteLater();
            m_activeTool = nullptr;
        }
    }
    m_zoomIndicator->hide();

    // Show the buttons after the resize of the selection or the creation
    // of a new one.
    if (!m_buttonHandler->isVisible() && m_selection->isVisible()) {
        // Don't go outside
        QRect newGeometry = m_selection->geometry().intersected(rect());
        // normalize
        if (newGeometry.width() <= 0) {
            int left = newGeometry.left();
            newGeometry.setLeft(newGeometry.right());
            newGeometry.setRight(left);
        }
        if (newGeometry.height() <= 0) {
            int top = newGeometry.top();
            newGeometry.setTop(newGeometry.bottom());
            newGeometry.setBottom(top);
        }
        m_selection->setGeometry(newGeometry);
        m_context.selection = extendedRect(&newGeometry);
        updateSizeIndicator();

        if((m_selection->x() + m_selection->width()/2 <= 265 )&& (m_selection->width() <= 530))
        {
            if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100 )&& (m_selection->height() <= 220))
                m_bar->move(m_selection->x(), m_selection->y() - 35);
            else if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100 )&& (m_selection->height() > 220))
                m_bar->move(m_selection->x(), m_selection->y() + m_selection->height() - 35);
            else
                m_bar->move(m_selection->x() , m_selection->y() + m_selection->height() + 2);
        }
        else if((m_selection->x() + m_selection->width()/2 >= m_context.desktop->screenGeometry().width() - 265 )&& (m_selection->width() <= 530))
        {
            if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100) && (m_selection->height() <= 265))
                m_bar->move(m_selection->x() + m_selection->width() - 530, m_selection->y() - 35);
            else if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100 )&& (m_selection->height() > 265))
                m_bar->move(m_selection->x() + m_selection->width() - 530, m_selection->y() + m_selection->height() - 35);
            else
                m_bar->move(m_selection->x() + m_selection->width() - 530, m_selection->y() + m_selection->height() + 2);
        }
        else
        {
            if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100) && (m_selection->height() <= 220))
                m_bar->move(m_selection->x() + m_selection->width()/2 - 265, m_selection->y() - 35);
            else if((m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100 )&& (m_selection->height() > 220))
                m_bar->move(m_selection->x() + m_selection->width()/2 - 265, m_selection->y() + m_selection->height() - 35);
            else
                m_bar->move(m_selection->x() + m_selection->width()/2 - 265 , m_selection->y() + m_selection->height() + 2);
        }

        m_buttonHandler->updatePosition(newGeometry);
        m_buttonHandler->show();
        m_bar->show();

        //updateToolBarPos();
    }
    m_mouseIsClicked = false;
    m_newSelection = false;
    m_grabbing = false;

    updateCursor();
}

void CaptureWidget::keyPressEvent(QKeyEvent *e) {
    if (!m_selection->isVisible()) {
        return;
    } else if (e->key() == Qt::Key_Up
               && m_selection->geometry().top() > rect().top()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() -1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    } else if (e->key() == Qt::Key_Down
               && m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() +1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    } else if (e->key() == Qt::Key_Left
               && m_selection->geometry().left() > rect().left()) {
        m_selection->move(QPoint(m_selection->x() -1, m_selection->y()));
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    } else if (e->key() == Qt::Key_Right
               && m_selection->geometry().right() < rect().right()) {
        m_selection->move(QPoint(m_selection->x() +1, m_selection->y()));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    } else if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = true;
    }
}

void CaptureWidget::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = false;
    }
}

//鼠标滚轮调整绘制线条大小
void CaptureWidget::wheelEvent(QWheelEvent *e) {
    m_context.thickness += e->delta() / 120;
    m_context.thickness = qBound(0, m_context.thickness, 100);
    QPoint topLeft = qApp->desktop()->screenGeometry(
                qApp->desktop()->screenNumber(QCursor::pos())).topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.thickness));
    if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    }
    emit thicknessChanged(m_context.thickness);
    emit thicknessCh(m_context.thickness);
}

void CaptureWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    m_context.widgetDimensions = rect();
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
    m_panel->setFixedHeight(height());
    if (!m_context.fullscreen) {
        m_buttonHandler->updateScreenRegions(rect());
    }
}

void CaptureWidget::moveEvent(QMoveEvent *e) {
    QWidget::moveEvent(e);
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
}

//初始化环境
void CaptureWidget::initContext(const QString &savePath, bool fullscreen) {
    m_context.widgetDimensions = rect();
    m_context.color = m_config.drawColorValue();
    m_context.savePath = savePath;
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
    m_context.mousePos= mapFromGlobal(QCursor::pos());
    m_context.thickness = m_config.drawThicknessValue();
    m_context.fullscreen = fullscreen;
    m_context.picture_format = m_config.PFormatValue();
    m_context.savetip = m_config.defaultSaveTip();
    m_context.desktop = QApplication::desktop();
}

//初始化panel和槽函数
void CaptureWidget::initPanel() {
    m_panel = new UtilityPanel(this);
    //b_panel = new ButtonPanel(this);
    makeChild(m_panel);
    QRect panelRect = rect();
    if (m_context.fullscreen) {
        panelRect = QGuiApplication::primaryScreen()->geometry();
    }
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    panelRect.setWidth(m_colorPicker->width() * 3);
    m_panel->setGeometry(panelRect);

   // b_panel->setGeometry(panelRect);

    SidePanelWidget *sidePanel =
            new SidePanelWidget(&m_context.screenshot);
    connect(sidePanel, &SidePanelWidget::colorChanged,
            this, &CaptureWidget::setDrawColor);
    /*connect(sidePanel, &SidePanelWidget::thicknessChanged,
            this, &CaptureWidget::setDrawThickness);*/
    connect(this, &CaptureWidget::colorChanged,
            sidePanel, &SidePanelWidget::updateColor);
    connect(this, &CaptureWidget::thicknessChanged,
            sidePanel, &SidePanelWidget::updateThickness);
    connect(sidePanel, &SidePanelWidget::togglePanel,
            m_panel, &UtilityPanel::toggle);
    sidePanel->colorChanged(m_context.color);
    sidePanel->thicknessChanged(m_context.thickness);
    m_panel->pushWidget(sidePanel);
    m_panel->pushWidget(new QUndoView(&m_undoStack, this));
}

void CaptureWidget::initSelection() {
    m_selection = new SelectionWidget(m_uiColor, this);
    connect(m_selection, &SelectionWidget::animationEnded, this, [this](){
        this->m_buttonHandler->updatePosition(this->m_selection->geometry());
    });
    m_selection->setVisible(false);
    m_selection->setGeometry(QRect());
}

void CaptureWidget::setState(CaptureButton *b) {
    if (!b) {
        return;
    }

    if (m_toolWidget) {
        m_toolWidget->deleteLater();
        if (m_activeTool->isValid()) {
            pushToolToStack();
        }
    }
    if (m_activeButton != b) {
        processTool(b->tool());
    }
    // Only close activated from button
    if (b->tool()->closeOnButtonPressed()) {
        close();
    }

    if (b->tool()->isSelectable()) {

        if (m_activeButton != b) {
            QWidget *confW = b->tool()->configurationWidget();
            m_panel->addToolWidget(confW);
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
            }
            m_activeButton = b;
            //按钮被点击，设置为天蓝色
            //m_activeButton->setColor(QColor (87, 250 , 255, 255));
            m_activeButton->setColor(Qt::gray);
           // qDebug() << b->tool()->name();

            if(b->tool()->name() == "选项")
            {
                m_menuController->showMenu(b->pos());
                m_colorPicker->hide();
                m_formtip->hide();
            }
            else
            {
                updateToolBar(b->tool()->name());
            }
            //m_menuController->showMenu(m_selection->getPoint());
        } else if (m_activeButton) {
            m_panel->clearToolWidget();
            m_activeButton->setColor(m_uiColor);
            m_activeButton = nullptr;
            m_colorPicker->hide();
            m_formtip->hide();
        }
        update(); // clear mouse preview
    }
}

void CaptureWidget::processTool(CaptureTool *t) {
    auto backup = m_activeTool;
    // The tool is active during the pressed().
    m_activeTool = t;
    t->pressed(m_context);
    m_activeTool = backup;
}

void CaptureWidget::handleButtonSignal(CaptureTool::Request r) {
    switch (r) {
    case CaptureTool::REQ_OPTION:
        setState(m_activeButton);
        saveScreenshot();
        //qDebug("111");
        break;
    case CaptureTool::REQ_CLEAR_MODIFICATIONS:
        m_undoStack.setIndex(0);
        update();
        break;
    case CaptureTool::REQ_CLOSE_GUI:
        close();
        break;
    case CaptureTool::REQ_HIDE_GUI:
        hide();
        break;
    case CaptureTool::REQ_HIDE_SELECTION:
        m_newSelection = true;
        m_selection->setVisible(false);
        updateCursor();
        break;
    case CaptureTool::REQ_SELECT_ALL:
        m_selection->setGeometryAnimated(rect());
        break;
    case CaptureTool::REQ_UNDO_MODIFICATION:
        m_undoStack.undo();
        break;
    case CaptureTool::REQ_REDO_MODIFICATION:
        m_undoStack.redo();
        break;
    case CaptureTool::REQ_REDRAW:
        update();
        break;
    case CaptureTool::REQ_TOGGLE_SIDEBAR:
        m_panel->toggle();
        break;
    case CaptureTool::REQ_SHOW_COLOR_PICKER:
        // TODO
        break;
    case CaptureTool::REQ_MOVE_MODE:
        setState(m_activeButton); // Disable the actual button
        break;
    case CaptureTool::REQ_CAPTURE_DONE_OK:
        m_captureDone = true;
        break;
    case CaptureTool::REQ_ADD_CHILD_WIDGET:
        if (!m_activeTool) {
            break;
        }
        if (m_toolWidget) {
            m_toolWidget->deleteLater();
        }
        m_toolWidget = m_activeTool->widget();
        if (m_toolWidget) {
            makeChild(m_toolWidget);
            m_toolWidget->move(m_context.mousePos);
            m_toolWidget->show();
            m_toolWidget->setFocus();
        }
        break;
    case CaptureTool::REQ_ADD_CHILD_WINDOW:
        if (!m_activeTool) {
            break;
        } else {
            QWidget *w = m_activeTool->widget();
            connect(this, &CaptureWidget::destroyed, w, &QWidget::deleteLater);
            w->show();
        }
        break;
    case CaptureTool::REQ_ADD_EXTERNAL_WIDGETS:
        if (!m_activeTool) {
            break;
        } else {
            QWidget *w = m_activeTool->widget();
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
        }
        break;
    default:
        break;
    }
}

void CaptureWidget::setDrawColor(const QColor &c) {
    m_context.color = c;
    ConfigHandler().setDrawColor(m_context.color);
    emit colorChanged(c);
}

void CaptureWidget::setDrawThickness(const int &t)
{
    m_context.thickness = qBound(0, t, 100);
    ConfigHandler().setdrawThickness(m_context.thickness);
    emit thicknessChanged(m_context.thickness);
    emit thicknessCh(m_context.thickness);
}

void CaptureWidget::leftResize() {
    if (m_selection->isVisible() && m_selection->geometry().right() > m_selection->geometry().left()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, -1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::rightResize() {
    if (m_selection->isVisible() && m_selection->geometry().right() < rect().right()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::upResize() {
    if (m_selection->isVisible() && m_selection->geometry().bottom() > m_selection->geometry().top()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 0, -1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::downResize() {
    if (m_selection->isVisible() && m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 0, 1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::initShortcuts() {
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(saveScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this, SLOT(copyScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(undo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z), this, SLOT(redo()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this, SLOT(rightResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this, SLOT(leftResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up), this, SLOT(upResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down), this, SLOT(downResize()));
    new QShortcut(Qt::Key_Space, this, SLOT(togglePanel()));
    new QShortcut(Qt::Key_Escape, this, SLOT(deleteToolwidgetOrClose()));
    new QShortcut(Qt::Key_Return, this, SLOT(saveScreenshot()));
    new QShortcut(Qt::Key_Enter, this, SLOT(saveScreenshot()));
}

void CaptureWidget::updateSizeIndicator() {
    if (m_sizeIndButton){
        const QRect &selection = extendedSelection();
        m_sizeIndButton->setText(QStringLiteral("%1\n%2")
                                     .arg(selection.width())
                                     .arg(selection.height()));
    }
}

void CaptureWidget::updateCursor() {
    if (m_rightClick) {
        setCursor(Qt::ArrowCursor);
    } else if (m_grabbing) {
        setCursor(Qt::ClosedHandCursor);
    } else if (!m_activeButton) {
        using sw = SelectionWidget;
        if (m_mouseOverHandle != sw::NO_SIDE){
            // cursor on the handlers
            switch (m_mouseOverHandle) {
            case sw::TOPLEFT_SIDE: case sw::BOTTONRIGHT_SIDE:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case sw::TOPRIGHT_SIDE: case sw::BOTTONLEFT_SIDE:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case sw::LEFT_SIDE: case sw::RIGHT_SIDE:
                setCursor(Qt::SizeHorCursor);
                break;
            case sw::TOP_SIDE: case sw::BOTTON_SIDE:
                setCursor(Qt::SizeVerCursor);
                break;
            default:
                break;
            }
        } else if (m_selection->isVisible() &&
                   m_selection->geometry().contains(m_context.mousePos))
        {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void CaptureWidget::pushToolToStack() {
    auto mod = new ModificationCommand(
                &m_context.screenshot, m_activeTool);
    disconnect(this, &CaptureWidget::colorChanged,
               m_activeTool, &CaptureTool::colorChanged);
    disconnect(this, &CaptureWidget::thicknessChanged,
               m_activeTool, &CaptureTool::thicknessChanged);
    if (m_panel->toolWidget()) {
        disconnect(m_panel->toolWidget(), nullptr, m_activeTool, nullptr);
    }
    m_undoStack.push(mod);
    m_activeTool = nullptr;
}

void CaptureWidget::makeChild(QWidget *w) {
    w->setParent(this);
    w->installEventFilter(m_eventFilter);
}

void CaptureWidget::togglePanel() {
    m_panel->toggle();
}

void CaptureWidget::childEnter() {
    m_previewEnabled = false;
    update();
}

void CaptureWidget::childLeave() {
    m_previewEnabled = true;
    update();
}

void CaptureWidget::copyScreenshot() {
    m_captureDone = true;
    ScreenshotSaver().saveToClipboard(pixmap());
    close();
}

void CaptureWidget::saveScreenshot() {
    m_captureDone = true;

    hide();
    //huoqu xitongcanshu
    //QString strDir = QDir::homePath();

    if(m_context.savetip == "desktip")
    {
        QString strDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        m_context.savePath = strDir;

        bool ok = ScreenshotSaver().saveToFilesystem(pixmap(), m_context.savePath, m_context.picture_format);
        if(ok)
            m_captureDone = true;
    }
    else if(m_context.savetip == "clicktip")
    {
        emit(copyScreenshot());
        return;
    }
    else if(m_context.savetip == "customtip")
    {
        bool ok = ScreenshotSaver().saveToFilesystemGUI(pixmap(),m_context.picture_format);
        if(ok)
            m_captureDone = true;
    }
    close();
}

void CaptureWidget::undo() {
    m_undoStack.undo();
}

void CaptureWidget::redo() {
    m_undoStack.redo();
}

QRect CaptureWidget::extendedSelection() const {
    if (!m_selection->isVisible())
        return QRect();
    QRect r = m_selection->geometry();
    return extendedRect(&r);
}

QRect CaptureWidget::extendedRect(QRect *r) const {
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
    return QRect(r->left()   * devicePixelRatio,
                 r->top()    * devicePixelRatio,
                 r->width()  * devicePixelRatio,
                 r->height() * devicePixelRatio);
}

void CaptureWidget::setPformat(QString format)
{
    m_context.picture_format = format;
    m_config.setPFormatValue(format);
    //qDebug() << m_context.picture_format;
}

void CaptureWidget::setSaveTip(QString saveTip)
{
    m_context.savetip = saveTip;
    m_config.setSaveTip(saveTip);
}

/*
void CaptureWidget::updateToolBarPos()
{
    QPoint toolbarPoint;
    toolbarPoint = QPoint(m_selection->x() + m_selection->width() - m_toolBar->width(),
                          std::max(m_selection->y() + m_selection->height() + TOOLBAR_Y_SPACING, 0));

    if (m_toolBar->width() > m_selection->x() + m_selection->width()) {
        toolbarPoint.setX(m_selection->x() + 8);
    }
    if (toolbarPoint.y()>= m_selection->y() + m_selection->height()
            - m_toolBar->height() - 28) {
        if (m_selection->y() > 28*2 + 10) {
            toolbarPoint.setY(m_selection->y() - m_toolBar->height() - TOOLBAR_Y_SPACING);
            qDebug("111");
        } else {
            toolbarPoint.setY(toolbarPoint.y());
        //}
    }
    m_toolBar->showAt(toolbarPoint);
}*/


void CaptureWidget::menuHide()
{
    setState(m_activeButton);
}

void CaptureWidget::updateToolBar(QString toolName)
{
    if(m_selection->y() + m_selection->height() >= m_context.desktop->screenGeometry().height() - 100)
    {
        m_colorPicker->move(m_bar->x() + m_activeButton->geometry().x() - 18, m_bar->y() - m_colorPicker->height() - 9);
        m_formtip->move(m_bar->x(),m_bar->y() - 9);
        isReverse = true;
    }
    else
    {
        m_colorPicker->move(m_bar->x() + m_activeButton->geometry().x() - 18, m_bar->y() + m_bar->height() + 9);
        m_formtip->move(m_bar->x(),m_bar->y() + m_bar->height());
        isReverse = false;
    }


    m_flag = true;
    //qDebug() << m_activeButton->mapToGlobal(*p);

    //m_activeButton->getRect();
    m_formtip->show(QPoint(m_activeButton->x() + m_activeButton->width()/2,m_activeButton->y()),isReverse);

    m_colorPicker->show(toolName);
    update();
}

void CaptureWidget::setDrawRectStyle(const int &s)
{
    m_context.graphic_style = s;
}

void CaptureWidget::setDrawLineStyle(const int &l)
{
    m_context.line_style = l;
}
