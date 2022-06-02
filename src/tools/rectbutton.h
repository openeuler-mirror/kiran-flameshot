#ifndef RECTBUTTON_H
#define RECTBUTTON_H

#include <QPushButton>
#include <QApplication>

class RectButton : public QPushButton {
    Q_OBJECT
public:
    RectButton(int style, QWidget* parent = 0) {
        Q_UNUSED(parent);
        setFixedSize(32, 26);
        setCheckable(true);
        m_tips = "";
        m_style = style;

         connect(this, &RectButton::clicked, this, &RectButton::setStyle);
    }
    ~RectButton(){}

public slots:
    void setTips(QString tips) {
        m_tips = tips;
    }

    QString getTips() {
        return m_tips;
    }

    void setStyle()
    {
        update();
        emit updateStyle(m_style);
    }

signals:
    void onEnter();
    void onExist();
    void updateStyle(int m_style);

protected:
    void enterEvent(QEvent* e) override {
        emit onEnter();
        QPushButton::enterEvent(e);
    }

    void leaveEvent(QEvent* e) override {
        emit onExist();
        QPushButton::leaveEvent(e);
    }

private:
    QString m_tips;
    int m_style;

};

#endif // RECTBUTTON_H
