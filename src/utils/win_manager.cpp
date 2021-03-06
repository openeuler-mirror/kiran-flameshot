//#include "win_manager.h"

//#include <QDebug>
//#include <QPixmap>
//#include <QIcon>
//#include <QRect>
//#include <QApplication>
//#include <QtX11Extras/QX11Info>

////#include <DObjectPrivate>

//#include <X11/Xatom.h>
//#include <X11/Xlib.h>

//MWindowManager::MWindowManager(QObject *parent) : QObject(parent)
//{
//    int screenNum;
//    conn = xcb_connect(0, &screenNum);
//    xcb_screen_t* screen = xcb_aux_get_screen(conn, screenNum);
//    rootWindow = screen->root;
//}

//MWindowManager::~MWindowManager()
//{
//    xcb_disconnect(conn);
//    conn = NULL;
//}

//xcb_atom_t MWindowManager::getAtom(QString name)
//{
//    QByteArray rawName = name.toLatin1();
//    xcb_atom_t result = XCB_ATOM_NONE;
//    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 0, rawName.size(), rawName.data());
//    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);
//    if(reply) {
//        result = reply->atom;

//        free(reply);
//    }

//    return result;
//}

//xcb_get_property_reply_t* MWindowManager::getProperty(xcb_window_t window, QString propertyName, xcb_atom_t type)
//{
//    xcb_get_property_cookie_t cookie = xcb_get_property(conn, 0, window, getAtom(propertyName), type, 0, UINT32_MAX);
//    return xcb_get_property_reply(conn, cookie, NULL);
//}

//QString MWindowManager::getAtomName(xcb_atom_t atom)
//{
//    QString result;

//    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(conn, atom);
//    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(conn, cookie, NULL);

//    if (reply) {
//        result = QString::fromLatin1(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
//        free(reply);
//    }

//    return result;
//}

//QStringList MWindowManager::getWindowTypes(xcb_window_t window)
//{
//    QStringList types;
//    xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_WINDOW_TYPE", XCB_ATOM_ATOM);

//    if(reply) {
//        xcb_atom_t *typesA = static_cast<xcb_atom_t*>(xcb_get_property_value(reply));
//        int typeNum = reply->length;

//        for(int i = 0; i < typeNum; i++) {
//            types.append(getAtomName(typesA[i]));
//        }

//        free(reply);
//    }

//    return types;
//}

//QStringList MWindowManager::getWindowStates(xcb_window_t window)
//{
//    QStringList types;
//    xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_STATE", XCB_ATOM_ATOM);

//    if(reply) {
//        xcb_atom_t *typesA = static_cast<xcb_atom_t*>(xcb_get_property_value(reply));
//        int typeNum = reply->length;

//        for(int i = 0; i < typeNum; i++) {
//            types.append(getAtomName(typesA[i]));
//        }

//        free(reply);
//    }

//    return types;
//}

//QList<int> MWindowManager::getWindowFrameExtents(xcb_window_t window)
//{
//    QList<int> extents;

//    if (window != rootWindow) {
//        xcb_get_property_reply_t *gtkFrameReply = getProperty(window, "_GTK_FRAME_EXTENTS", XCB_ATOM_CARDINAL);

//        if (gtkFrameReply) {
//            // Because XCB haven't function to check property is exist,
//            // we will got wrong value if property '_GTK_FRAME_EXTENTS' is not exist in window xprop attributes.
//            // So we check reply->format, if it equal 16 or 32, '_GTK_FRAME_EXTENTS' property is exist.
//            if (gtkFrameReply->format == 32 || gtkFrameReply->format == 16) {
//                int32_t *value = (int32_t *)xcb_get_property_value(gtkFrameReply);
//                for (int i = 0; i < 4; ++i) {
//                    extents.append(value[i]);
//                }
//            } else {
//                for (int j = 0; j < 4; ++j) {
//                    extents.append(0);
//                }
//            }
//        }

//        free(gtkFrameReply);
//    }

//    return extents;
//}

//QString MWindowManager::getWindowClass(xcb_window_t window)
//{
//    if (window == rootWindow) {
//        return tr("Desktop");
//    } else {
//        xcb_get_property_reply_t *reply = getProperty(window, "WM_CLASS", getAtom("STRING"));

//        if(reply) {
//            QList<QByteArray> rawClasses = QByteArray(static_cast<char*>(xcb_get_property_value(reply)),
//                                                      xcb_get_property_value_length(reply)).split('\0');

//            free(reply);

//            return QString::fromLatin1(rawClasses[0]);
//        } else {
//            return QString();
//        }
//    }
//}

//QString MWindowManager::getWindowFlatpakAppid(xcb_window_t window)
//{
//    xcb_get_property_reply_t *reply = getProperty(window, "FLATPAK_APPID", getAtom("STRING"));

//    if(reply) {
//        QList<QByteArray> rawClasses = QByteArray(static_cast<char*>(xcb_get_property_value(reply)),
//                                                  xcb_get_property_value_length(reply)).split('\0');

//        free(reply);

//        return QString::fromLatin1(rawClasses[0]);
//    } else {
//        return QString();
//    }
//}

//QString MWindowManager::getWindowName(xcb_window_t window)
//{
//    if (window == rootWindow) {
//        return tr("Desktop");
//    } else {
//        xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_NAME", getAtom("UTF8_STRING"));

//        if(reply) {
//            QString result = QString::fromUtf8(static_cast<char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply));

//            free(reply);

//            return result;
//        } else {
//            return QString();
//        }
//    }
//}

//int MWindowManager::getCurrentWorkspace(xcb_window_t window)
//{
//    xcb_get_property_reply_t *reply = getProperty(window, "_NET_CURRENT_DESKTOP", XCB_ATOM_CARDINAL);
//    int desktop = 0;

//    if (reply) {
//        desktop = *((int *) xcb_get_property_value(reply));

//        free(reply);
//    }

//    return desktop;
//}

//int MWindowManager::getWindowWorkspace(xcb_window_t window)
//{
//    if (window == rootWindow) {
//        return getCurrentWorkspace(rootWindow);
//    } else {
//        xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_DESKTOP", XCB_ATOM_CARDINAL);
//        int desktop = 0;

//        if (reply) {
//            desktop = *((int *) xcb_get_property_value(reply));

//            free(reply);
//        }

//        return desktop;
//    }
//}

//QList<xcb_window_t> MWindowManager::getWindows()
//{
//    QList<xcb_window_t> windows;
//    xcb_get_property_reply_t *listReply = getProperty(rootWindow, "_NET_CLIENT_LIST_STACKING", XCB_ATOM_WINDOW);

//    if (listReply) {
//        xcb_window_t *windowList = static_cast<xcb_window_t*>(xcb_get_property_value(listReply));
//        int windowListLength = listReply->length;

//        for (int i = 0; i < windowListLength; i++) {
//            xcb_window_t window = windowList[i];

//            Q_FOREACH(QString type, getWindowTypes(window)) {
//                if (type == "_NET_WM_WINDOW_TYPE_NORMAL" ||
//                    type == "_NET_WM_WINDOW_TYPE_DIALOG"
//                    ) {
//                    bool needAppend = false;

//                    QStringList states = getWindowStates(window);
//                    if (states.length() == 0 ||
//                        (!states.contains("_NET_WM_STATE_HIDDEN"))) {
//                        if (getWindowWorkspace(window) == getCurrentWorkspace(rootWindow)) {
//                            needAppend = true;
//                        }
//                    }

//                    if (needAppend) {
//                        windows.append(window);
//                        break;
//                    }
//                }
//            }
//        }

//        free(listReply);
//    }

//    // We need re-sort windows list from up to bottom,
//    // to make compare cursor with window area from up to bottom.
//    std::reverse(windows.begin(), windows.end());

//    // Add desktop window.
//    windows.append(rootWindow);

//    // Just use for debug.
//    // foreach (auto window, windows) {
//    //     qDebug() << getWindowName(window);
//    // }

//    return windows;
//}

//void MWindowManager::setRootWindowRect(QRect rect) {
//    rootWindowRect.x = rect.x();
//    rootWindowRect.y = rect.y();
//    rootWindowRect.width = rect.width();
//    rootWindowRect.height = rect.height();
//}

//WindowRect MWindowManager::getRootWindowRect() {

//    return rootWindowRect;
//}

//void MWindowManager::translateCoords(xcb_window_t window, int32_t& x, int32_t& y)
//{
//    xcb_translate_coordinates_cookie_t c = xcb_translate_coordinates(conn,
//                                                                     rootWindow, window, x, y);
//    xcb_translate_coordinates_reply_t* reply = xcb_translate_coordinates_reply(conn, c, NULL);

//    if (reply) {
//        x = reply->dst_x;
//        y = reply->dst_y;

//        free(reply);
//    }
//}

//WindowRect MWindowManager::getWindowRect(xcb_window_t window)
//{
//    WindowRect rect;

//    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn,
//                                                                xcb_get_geometry(conn, window), 0);
//    xcb_translate_coordinates_reply_t *coordinate = xcb_translate_coordinates_reply(conn,
//        xcb_translate_coordinates(conn, window, rootWindow, 0, 0), 0);

//    QList<int> extents = getWindowFrameExtents(window);

//    rect.x = coordinate->dst_x;
//    rect.y = coordinate->dst_y;
//    rect.width = geometry->width;
//    rect.height = geometry->height;

//    if (extents.length() == 4) {
//        // _GTK_FRAME_EXTENTS: left, right, top, bottom
//        rect.x += extents[0];
//        rect.y += extents[2];
//        rect.width -= extents[0] + extents[1];
//        rect.height -= extents[2] + extents[3];
//    }

//    const qreal ratio = qApp->devicePixelRatio();
//    if (ratio != 1.0) {
//        rect.x /= ratio;
//        rect.y /= ratio;
//        rect.width /= ratio;
//        rect.height /= ratio;
//    }

//    free(geometry);
//    free(coordinate);

//    return rect;
//}

//WindowRect MWindowManager::adjustRectInScreenArea(WindowRect rect)
//{
//    WindowRect rootWindowRect = getRootWindowRect();

//    WindowRect newRect;
//    newRect.x = rect.x >= rootWindowRect.x ? rect.x : rootWindowRect.x;
//    newRect.y = rect.y >= rootWindowRect.y ? rect.y : rootWindowRect.y;
//    newRect.width = rect.x >= rootWindowRect.x ? rect.width : rect.width + (rect.x - rootWindowRect.x);
//    newRect.height = rect.y >= rootWindowRect.y ? rect.height : rect.height + (rect.y - rootWindowRect.y);

//    if (newRect.x + newRect.width > rootWindowRect.x + rootWindowRect.width) {
//        newRect.width = rootWindowRect.x + rootWindowRect.width - newRect.x;
//    }

//    if (newRect.y + newRect.height > rootWindowRect.y + rootWindowRect.height) {
//        newRect.height = rootWindowRect.y + rootWindowRect.height - newRect.y;
//    }

//    return newRect;
//}

//template <typename... ArgTypes, typename... ArgTypes2>
//static inline unsigned int XcbCallVoid(xcb_void_cookie_t (*func)(xcb_connection_t *, ArgTypes...), ArgTypes2... args...)
//{
//    return func(QX11Info::connection(), args...).sequence;
//}

//void MWindowManager::setWindowBlur(int wid, QVector<uint32_t> &data)
//{
//    xcb_atom_t atom = getAtom("_NET_WM_DEEPIN_BLUR_REGION_ROUNDED");
//    XcbCallVoid(
//        xcb_change_property,
//        XCB_PROP_MODE_REPLACE,
//        wid,
//        atom,
//        XCB_ATOM_CARDINAL,
//        32,
//        data.size(),
//        data.constData());
//    xcb_flush(conn);
//}

//int MWindowManager::getWindowPid(xcb_window_t window)
//{
//    xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_PID", XCB_ATOM_CARDINAL);
//    int pid = 0;

//    if (reply) {
//        pid = *((int *) xcb_get_property_value(reply));

//        free(reply);
//    }

//    return pid;
//}

//QPixmap MWindowManager::getWindowIcon(xcb_window_t win, int iconSize)
//{
//    QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");
//    QPixmap defaultPixmap = defaultExecutableIcon.pixmap(iconSize, iconSize);

//    if (win > 0) {
//        int format;
//        ulong type, nitems, extra;
//        ulong* data = 0;

//        XGetWindowProperty(QX11Info::display(), win, getAtom("_NET_WM_ICON"),
//                           0, LONG_MAX, False, AnyPropertyType,
//                           &type, &format, &nitems, &extra,
//                           (uchar**)&data);
//        if (!data) {
//            return defaultPixmap;
//        }

//        QImage img (data[0], data[1], QImage::Format_ARGB32);
//        for (int i=0; i<img.byteCount()/4; ++i) {
//            ((uint*)img.bits())[i] = data[i+2];
//        }

//        // Need add options Qt::KeepAspectRatio and Qt::SmoothTransformation to keep window icon scaled smoothly.
//        QPixmap pixmap = QPixmap::fromImage(img).scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        XFree(data);

//        return pixmap;
//    }

//    return defaultPixmap;
//}

//void MWindowManager::killWindow(xcb_window_t window)
//{
//    XKillClient(QX11Info::display(), window);
//    XSync(QX11Info::display(), 0);
//}
