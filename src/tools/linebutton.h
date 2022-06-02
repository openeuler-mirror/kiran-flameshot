#ifndef LINEBUTTON_H
#define LINEBUTTON_H

#include <QPushButton>
#include <QApplication>

class LineButton : public QPushButton {
    Q_OBJECT
public:
    LineButton(int line, QWidget* parent = 0) {
        Q_UNUSED(parent);
        setFixedSize(32, 26);
        setCheckable(true);
        m_tips = "";
        m_line = line;

         connect(this, &LineButton::clicked, this, &LineButton::setLine);
    }
    ~LineButton(){}

public slots:
    void setTips(QString tips) {
        m_tips = tips;
    }

    QString getTips() {
        return m_tips;
    }

    void setLine()
    {
        update();
        emit updateLine(m_line);
    }

signals:
    void onEnter();
    void onExist();
    void updateLine(int m_style);

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
    int m_line;

};

#endif // LINEBUTTON_H
