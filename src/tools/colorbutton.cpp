#include "colorbutton.h"

#include <QPainter>
#include <QDebug>

const QSize BUTTON_SIZE = QSize(16, 16);
const int  ELLIPSE_MARGIN = 1;
ColorButton::ColorButton(QColor bgColor, QWidget *parent)
    : QPushButton(parent) {
    setFixedSize(BUTTON_SIZE);
    setCheckable(true);
    m_bgColor = bgColor;

    connect(this, &ColorButton::clicked, this, &ColorButton::setColorBtnChecked);
}

void ColorButton::setColorBtnChecked() {
    update();
    if (this->isChecked()) {
        qDebug() << "updatePaintColor:" << m_bgColor;
        emit updatePaintColor(m_bgColor);
    }
}

QColor ColorButton::getColor()
{
    return m_bgColor;
}

ColorButton::~ColorButton() {}
