#ifndef THICKNESSWIDGET_H
#define THICKNESSWIDGET_H

#pragma once

#include <QWidget>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class ThicknessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThicknessWidget(int thickness, QWidget *parent = nullptr);

public slots:
    void updateThickness(int size);
    void hide();
    void show();

signals:
    void thicknessSelected(int size);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    QVector<QRect> handleMask() const;

private:
    int m_colorAreaSize;

    QVector<QRect> m_thinknessAreaList;
    QVector<int> m_thinknessList;

    int m_thinkness, m_drawThinkness, m_selectThinkness;

    bool m_inselection = false;
    bool m_ishide = false;
};

#endif // THICKNESSWIDGET_H


