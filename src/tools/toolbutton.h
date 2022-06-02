#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QPushButton>
#include <QApplication>

class ToolButton : public QPushButton {
    Q_OBJECT
public:
    ToolButton(int thickness, QWidget* parent = 0) {
        Q_UNUSED(parent);
        setFixedSize(32, 26);
        setCheckable(true);
        m_tips = "";
        m_thickness = thickness;

         connect(this, &ToolButton::clicked, this, &ToolButton::setThickness);
    }
    ~ToolButton(){}

    int m_thickness;

public slots:
    void setTips(QString tips) {
        m_tips = tips;
    }

    QString getTips() {
        return m_tips;
    }

    void setThickness()
    {
        update();
        emit updateThickness(m_thickness);
    }

signals:
    void onEnter();
    void onExist();
    void updateThickness(int thickness);

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


};
#endif // TOOLBUTTON_H
