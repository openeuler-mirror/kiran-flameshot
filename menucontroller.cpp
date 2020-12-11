/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "menucontroller.h"
#include "configsettings.h"
#include "saveutils.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <map>

const QSize MENU_ICON_SIZE = QSize(14, 14);

MenuController::MenuController(QObject *parent)
    : QObject(parent)
    , m_ration(1)
{
    m_menu = new QMenu;
    m_menu->setFocusPolicy(Qt::StrongFocus);
    m_menu->setStyle(QStyleFactory::create("dlight"));
    QString qss = "QMenu{background-color: rgb(255, 255, 255);"
                           "border-radius: 20px; "
                           "}"
                           "QMenu::item { background-color: transparent;"
                           "padding:3px 20px;"
                           "margin:5px 10px;"
                           "}"
                           "QMenu::item:selected { background-color: gray; }";
    m_menu->setStyleSheet(qss);

    pathtip = new QAction(tr("save to"));
    desktip = new QAction(tr("desktop"));
    clicktip = new QAction(tr("cilickborad"));
    customtip = new QAction(tr("Custom save location"));

    desktip->setCheckable(true);
    clicktip->setCheckable(true);
    customtip->setCheckable(true);

    /*QAction* savetip = new QAction(tr("保存方式"));
    QAction* pathtip = new QAction(tr("存储到"));
    QAction* pathtip = new QAction(tr("存储到"));
    QAction* pathtip = new QAction(tr("存储到"));*/

    formattip = new QAction(tr("保存格式"));
    jpgtip = new QAction(tr("JPG"));
    pngtip = new QAction(tr("PNG"));
    bmptip = new QAction(tr("BMP"));

    jpgtip->setCheckable(true);
    pngtip->setCheckable(true);
    bmptip->setCheckable(true);

    pathtip->setEnabled(false);
    //savetip->setEnabled(false);
    formattip->setEnabled(false);

    m_menu->addAction(pathtip);
    m_menu->addAction(desktip);
    m_menu->addAction(clicktip);
    m_menu->addAction(customtip);
    //m_menu->addAction(savetip);
    m_menu->addAction(formattip);
    m_menu->addAction(jpgtip);
    m_menu->addAction(pngtip);
    m_menu->addAction(bmptip);

    //set default setting
    /*desktip->setChecked(true);
    jpgtip->setChecked(true);*/

    connect(m_menu, &QMenu::aboutToHide,[=]{
        emit menuHide();
    });

    connect(desktip, &QAction::triggered, [=]{
        emit on_desktip_triggered();
    });
    connect(clicktip, &QAction::triggered, [=]{
        emit on_clicktip_triggered();
    });
    connect(customtip, &QAction::triggered, [=]{
        emit on_customtip_triggered();
    });


    connect(jpgtip, &QAction::triggered, [=]{
        emit on_jpgtip_triggered();
    });
    connect(pngtip, &QAction::triggered, [=]{
        emit on_pngtip_triggered();
    });
    connect(bmptip, &QAction::triggered, [=]{
        emit on_bmptip_triggered();
    });

    /*QAction* pathtip = new QAction(tr("存储到"));
    QAction* savetip = new QAction(tr("保存方式"));
    QAction* formattip = new QAction(tr("格式"));
    QAction* pathtip = new QAction(tr("存储到"));
    QAction* savetip = new QAction(tr("保存方式"));
    QAction* formattip = new QAction(tr("格式"));
    QAction* formattip = new QAction(tr("格式"));*/

    /*m_menu->addAction(rectAct);
    m_menu->addAction(ovalAct);
    m_menu->addAction(arrowAct);
    m_menu->addAction(penAct);
    m_menu->addAction(textAct);
    m_menu->addSeparator();
    m_menu->addAction(unDoAct);*/

    /*QIcon saveIcon;
    saveIcon.addFile(":/img/menu_icons/save-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
    saveIcon.addFile(":/img/menu_icons/save-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
    QMenu* saveMenu =  m_menu->addMenu(saveIcon, tr("Save"));

    saveMenu->setStyle(QStyleFactory::create("dlight"));
    QAction* saveAct1 = new QAction(tr("Save to desktop"), this);
    QAction* saveAct2 = new QAction(tr("Autosave"), this);
    QAction* saveAct3 = new QAction(tr("Save to specified folder"), this);
    QAction* saveAct4 = new QAction(tr("Copy to clipboard"), this);
    QAction* saveAct5 = new QAction(tr("Autosave and copy to clipboard"), this);

    std::map<SaveAction, QAction*> actionMaps;
    actionMaps[SaveAction::SaveToDesktop] = saveAct1;
    actionMaps[SaveAction::AutoSave] = saveAct2;
    actionMaps[SaveAction::SaveToSpecificDir] = saveAct3;
    actionMaps[SaveAction::SaveToClipboard] = saveAct4;
    actionMaps[SaveAction::SaveToAutoClipboard] = saveAct5;*/

    /*SaveAction action = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();

    for (auto it = actionMaps.begin(); it != actionMaps.end(); ++it) {
        saveMenu->addAction(it->second);

        connect(it->second, &QAction::triggered, this, [=] {
            emit saveBtnPressed(action);
        });

        if (action == it->first) {
            it->second->setCheckable(true);
            it->second->setChecked(true);
        }
    }

    QIcon exitIcon;
    exitIcon.addFile(":/img/menu_icons/exit-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
    exitIcon.addFile(":/img/menu_icons/exit-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
    QAction* closeAct = new QAction(exitIcon, tr("Exit"), this);
    m_menu->addAction(closeAct);
    connect(closeAct, &QAction::triggered, this, [=]{
        emit shapePressed("close");
    });*/


   // connect(desktip, &QAction::triggered, this, SLOT(on_desktip_triggered()));
}

QMenu *MenuController::getMenu() {
    return m_menu;
}

void MenuController::showMenu(QPoint pos) {
    m_menu->popup(pos);
}

void MenuController::hide() {
    m_menu->hide();
}

void MenuController::on_desktip_triggered()
{
    clicktip->setChecked(false);
    customtip->setChecked(false);
    emit setSaveTip("desktip");
}

void MenuController::on_clicktip_triggered()
{
    desktip->setChecked(false);
    customtip->setChecked(false);
    emit setSaveTip("clicktip");
}

void MenuController::on_customtip_triggered()
{
    clicktip->setChecked(false);
    desktip->setChecked(false);
    emit setSaveTip("customtip");
}

void MenuController::on_jpgtip_triggered()
{
    pngtip->setChecked(false);
    bmptip->setChecked(false);
    emit setPformat("jpg");
}

void MenuController::on_pngtip_triggered()
{
    jpgtip->setChecked(false);
    bmptip->setChecked(false);
    emit setPformat("png");
}

void MenuController::on_bmptip_triggered()
{
    jpgtip->setChecked(false);
    pngtip->setChecked(false);
    emit setPformat("bmp");
}

void MenuController::setDefaultFormat(QString pformat)
{
    if(pformat == "jpg")
        jpgtip->setChecked(true);
    else if(pformat == "png")
        pngtip->setChecked(true);
    else if(pformat == "bmp")
        bmptip->setChecked(true);
}

void MenuController::setDefaultCheck(QString saveTip){
    if(saveTip == "desktip")
        desktip->setChecked(true);
    else if(saveTip == "clicktip")
        clicktip->setChecked(true);
    else if(saveTip == "customtip")
    {
        customtip->setChecked(true);
    }
}

MenuController::~MenuController() {

}
