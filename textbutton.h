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

#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

//#include <QPushButton>
#include <QComboBox>
#include <QApplication>
#include <baseutils.h>

class TextButton : public QComboBox {
    Q_OBJECT
public:
    TextButton(int thickness, QWidget* parent = 0) {
        Q_UNUSED(parent);
         setObjectName("TextButton");
         m_thickness = thickness;
         addItem(tr("small"));
         addItem(tr("medium"));
         addItem(tr("large"));

         if(m_thickness = SMALL_SIZE)
             this->setCurrentIndex(0);
         else if(m_thickness = MEDIUM_SIZE)
             this->setCurrentIndex(1);
         else if(m_thickness = LARGE_SIZE)
             this->setCurrentIndex(2);


         setFixedSize(24, 26);

         connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(setThickness(int)));
    }
    ~TextButton(){}

    int m_thickness;

public slots:
    void setTips(QString tips) {
        m_tips = tips;
    }

    QString getTips() {
        return m_tips;
    }

    void setThickness(int index)
    {
        if(this->currentIndex() == 0)
            m_thickness = SMALL_SIZE;
        else if(this->currentIndex() == 1)
            m_thickness = MEDIUM_SIZE;
        else if(this->currentIndex() == 2)
            m_thickness = LARGE_SIZE;
//        printf("index:%d\n",this->currentIndex());
//        printf("dididi:%d\n",m_thickness);
        emit updateThickness(m_thickness);
    }

signals:
    void onEnter();
    void onExist();
    void updateThickness(int thickness);

protected:
    void enterEvent(QEvent* e) override {
        emit onEnter();
        QComboBox::enterEvent(e);
    }

    void leaveEvent(QEvent* e) override {
        emit onExist();
        QComboBox::leaveEvent(e);
    }

private:
    QString m_tips;


};

#endif // TEXTBUTTON_H


