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

#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <QMenu>
#include <QPropertyAnimation>

enum SaveAction:unsigned int;
class MenuController : public QObject {
    Q_OBJECT
public:
    MenuController(QObject* parent = 0);
    ~MenuController();

signals:
    void shapePressed(QString currentShape);
    void unDoAction();
    void saveBtnPressed(SaveAction action);
    void menuNoFocus();
    void setPformat(QString format);
    void setSaveTip(QString saveTip);
    void menuHide();

private slots:
    void on_desktip_triggered();
    void on_clicktip_triggered();
    void on_customtip_triggered();

    void on_jpgtip_triggered();
    void on_pngtip_triggered();
    void on_bmptip_triggered();

public slots:
    void showMenu(QPoint pos);
    void hide();
    QMenu *getMenu();
    void setDefaultFormat(QString pformat);
    void setDefaultCheck(QString saveTip);

private:
    QMenu* m_menu;
    qreal m_ration;

    QAction* pathtip;
    QAction* desktip;
    QAction* clicktip;
    QAction* customtip;

    QAction* formattip;
    QAction* jpgtip;
    QAction* pngtip;
    QAction* bmptip;
};
#endif // MENUCONTROLLER_H
