#ifndef KWINDOWMANAGER_H
#define KWINDOWMANAGER_H

#include <QObject>


struct WindowRect {
    int x;
    int y;
    int width;
    int height;
};

class Kwindowmanager : public QObject
{
    Q_OBJECT
public:
    explicit Kwindowmanager(QObject *parent = nullptr);
    ~Kwindowmanager();


signals:

public slots:

private:
};

#endif // KWINDOWMANAGER_H
