#ifndef FONTWIDGET_H
#define FONTWIDGET_H

#include "textbutton.h"
#include <QWidget>

class FontWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FontWidget(int thickness, QWidget *parent = nullptr);

public slots:
    void updateThickness(int size);
    void thicknessChanged(int size);

signals:
    void thicknessSelected(int size);

protected:
    void initTextLabel();

private:
    int m_colorAreaSize;

    QVector<QRect> m_thinknessAreaList;
    QVector<int> m_thinknessList;

    int m_thinkness, m_drawThinkness, m_selectThinkness;

    bool m_inselection = false;

    QWidget* m_textLabel;

    TextButton *textButton;
};

#endif // FONTWIDGET_H
