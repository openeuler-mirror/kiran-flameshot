#ifndef FORMTIP_H
#define FORMTIP_H

#include <QWidget>

class FormTip : public QWidget
{
    Q_OBJECT
public:
    explicit FormTip(QWidget *parent = nullptr);

    void show(const QPoint p, bool isReverse);
    void hide();

protected:
    void paintEvent(QPaintEvent *);

private:
    QPoint point;
    bool m_isReverse = false;
};

#endif // FORMTIP_H
