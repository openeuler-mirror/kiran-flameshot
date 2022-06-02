//#ifndef TEST_H
//#define TEST_H

//#pragma once

//#include <QObject>

////#include <DObject>


//#include <xcb/xcb.h>
//#include <xcb/xcb_aux.h>



////#include "dtkwm_global.h"

//struct WindowRect {
//    int x;
//    int y;
//    int width;
//    int height;
//};


//class MWindowManager : public QObject{
//    Q_OBJECT

//public:
//    explicit MWindowManager(QObject *parent = nullptr);
//    ~MWindowManager();

//    QList<int> getWindowFrameExtents(xcb_window_t window);
//    QList<xcb_window_t> getWindows();
//    QString getAtomName(xcb_atom_t atom);
//    QString getWindowName(xcb_window_t window);
//    QString getWindowClass(xcb_window_t window);
//    QString getWindowFlatpakAppid(xcb_window_t window);
//    QStringList getWindowTypes(xcb_window_t window);
//    QStringList getWindowStates(xcb_window_t window);
//    WindowRect getRootWindowRect();
//    void setRootWindowRect(QRect rect);
//    WindowRect getWindowRect(xcb_window_t window);
//    int getCurrentWorkspace(xcb_window_t window);
//    int getWindowWorkspace(xcb_window_t window);
//    xcb_atom_t getAtom(QString name);
//    xcb_get_property_reply_t* getProperty(xcb_window_t window, QString propertyName, xcb_atom_t type);
//    void setWindowBlur(int wid, QVector<uint32_t> &data);
//    void translateCoords(xcb_window_t window, int32_t& x, int32_t& y);
//    WindowRect adjustRectInScreenArea(WindowRect rect);
//    int getWindowPid(xcb_window_t window);
//    QPixmap getWindowIcon(xcb_window_t win, int iconSize);
//    void killWindow(xcb_window_t window);

//    xcb_window_t rootWindow;

//    xcb_connection_t* conn;
//    WindowRect rootWindowRect;
//};

//#endif
