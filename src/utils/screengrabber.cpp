// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "screengrabber.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QCursor>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QX11Info>
#include <QPainter>
// X11 头文件放在 Qt 头文件之后，避免 Xlib 宏（None/Bool/Status 等）污染 Qt 头
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#endif

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent) {

}

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
namespace {

// 读窗口属性；失败或类型/format 不符时返回 false（成功时调用方 XFree data）
bool getWindowProperty(Display *dpy, Window win, Atom prop, Atom reqType,
                       int reqFormat, unsigned long minItems,
                       Atom *actualType, int *actualFormat,
                       unsigned long *nItems, unsigned char **data)
{
    unsigned long bytesAfter = 0;
    *data = nullptr;
    int status = XGetWindowProperty(dpy, win, prop, 0, (~0L) >> 2, False,
                                    reqType, actualType, actualFormat,
                                    nItems, &bytesAfter, data);
    if (status != Success || !*data) {
        return false;
    }
    if (*actualType != reqType || *actualFormat != reqFormat || *nItems < minItems) {
        XFree(*data);
        *data = nullptr;
        return false;
    }
    return true;
}

Window readNetActiveWindow(Display *dpy, Window root)
{
    // 要求 type=WINDOW、format=32、id≠0
    Atom atom = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (atom == None) {
        return None;
    }
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nItems = 0;
    unsigned char *data = nullptr;
    if (!getWindowProperty(dpy, root, atom, XA_WINDOW, 32, 1,
                           &actualType, &actualFormat, &nItems, &data)) {
        return None;
    }
    Window active = *reinterpret_cast<Window *>(data);
    XFree(data);
    return active;
}

void readFrameExtents(Display *dpy, Window win,
                      int *left, int *right, int *top, int *bottom)
{
    // 缺失或非法时 extents 置 0，不报错
    *left = *right = *top = *bottom = 0;
    Atom atom = XInternAtom(dpy, "_NET_FRAME_EXTENTS", True);
    if (atom == None) {
        return;
    }
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nItems = 0;
    unsigned char *data = nullptr;
    if (!getWindowProperty(dpy, win, atom, XA_CARDINAL, 32, 4,
                           &actualType, &actualFormat, &nItems, &data)) {
        return;
    }
    long *ext = reinterpret_cast<long *>(data);
    *left = static_cast<int>(ext[0]);
    *right = static_cast<int>(ext[1]);
    *top = static_cast<int>(ext[2]);
    *bottom = static_cast<int>(ext[3]);
    XFree(data);
}

bool windowTypeAllowed(Display *dpy, Window win)
{
    // 列表含 NORMAL 或 DIALOG，且不含 DESKTOP/DOCK → 允许；无 TYPE → 当 NORMAL。
    // 允许 OVERRIDE+NORMAL：自绘标题栏（如控制面板）常把
    // _KDE_NET_WM_WINDOW_TYPE_OVERRIDE 与 _NET_WM_WINDOW_TYPE_NORMAL 同列；
    // OVERRIDE 仅表示无 WM 边框，不应单独否决。
    Atom typeAtom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", True);
    if (typeAtom == None) {
        return true;
    }
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nItems = 0;
    unsigned long bytesAfter = 0;
    unsigned char *data = nullptr;
    int status = XGetWindowProperty(dpy, win, typeAtom, 0, (~0L) >> 2, False,
                                    XA_ATOM, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data);
    if (status != Success || !data || nItems == 0) {
        if (data) {
            XFree(data);
        }
        // 无属性或空列表：当 NORMAL
        return true;
    }
    if (actualType != XA_ATOM || actualFormat != 32) {
        XFree(data);
        return true;
    }

    Atom atomNormal = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    Atom atomDialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    Atom atomDesktop = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    Atom atomDock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom *types = reinterpret_cast<Atom *>(data);
    bool hasAllowed = false;
    bool hasDenied = false;
    for (unsigned long i = 0; i < nItems; ++i) {
        if (types[i] == atomDesktop || types[i] == atomDock) {
            hasDenied = true;
            break;
        }
        if (types[i] == atomNormal || types[i] == atomDialog) {
            hasAllowed = true;
        }
    }
    XFree(data);
    return hasAllowed && !hasDenied;
}

bool windowIsHidden(Display *dpy, Window win)
{
    // _NET_WM_STATE_HIDDEN 则视为不可截取
    Atom stateAtom = XInternAtom(dpy, "_NET_WM_STATE", True);
    Atom hiddenAtom = XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", True);
    if (stateAtom == None || hiddenAtom == None) {
        return false;
    }
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nItems = 0;
    unsigned long bytesAfter = 0;
    unsigned char *data = nullptr;
    int status = XGetWindowProperty(dpy, win, stateAtom, 0, (~0L) >> 2, False,
                                    XA_ATOM, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data);
    if (status != Success || !data || nItems == 0 ||
            actualType != XA_ATOM || actualFormat != 32) {
        if (data) {
            XFree(data);
        }
        return false;
    }
    Atom *states = reinterpret_cast<Atom *>(data);
    bool hidden = false;
    for (unsigned long i = 0; i < nItems; ++i) {
        if (states[i] == hiddenAtom) {
            hidden = true;
            break;
        }
    }
    XFree(data);
    return hidden;
}

// 非 0、InputOutput、IsViewable；BadWindow → 失败
bool isValidCaptureTarget(Display *dpy, Window win)
{
    if (win == None) {
        return false;
    }
    XWindowAttributes attrs;
    if (!XGetWindowAttributes(dpy, win, &attrs)) {
        return false;
    }
    if (attrs.c_class != InputOutput) {
        return false;
    }
    if (attrs.map_state != IsViewable) {
        return false;
    }
    if (windowIsHidden(dpy, win)) {
        return false;
    }
    if (!windowTypeAllowed(dpy, win)) {
        return false;
    }
    return true;
}

} // namespace
#endif

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, bool includeCursor) {
    ok = true;
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if(m_info.waylandDectected()) {
        QPixmap res;
        // handle screenshot based on DE
        switch (m_info.windowManager()) {
        case DesktopInfo::GNOME: {
            // https://github.com/GNOME/gnome-shell/blob/695bfb96160033be55cfb5ac41c121998f98c328/data/org.gnome.Shell.Screenshot.xml
            QString path = FileNameHandler().generateAbsolutePath(QDir::tempPath()) + ".png";
            QDBusInterface gnomeInterface(QStringLiteral("org.gnome.Shell"),
                                          QStringLiteral("/org/gnome/Shell/Screenshot"),
                                          QStringLiteral("org.gnome.Shell.Screenshot"));
            QDBusReply<bool> reply = gnomeInterface.call(QStringLiteral("Screenshot"), includeCursor, false, path);
            if (reply.value()) {
                res = QPixmap(path);
                QFile dbusResult(path);
                dbusResult.remove();
            } else {
                ok = false;
            }
            break;
        } case DesktopInfo::KDE: {
            // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
            QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Screenshot"),
                                         QStringLiteral("org.kde.kwin.Screenshot"));
            // KWin 支持 captureCursor 参数
            QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"), includeCursor);
            if (!reply.isValid()) {
                // 降级：旧版接口无参数
                reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            }
            res = QPixmap(reply.value());
            if (!res.isNull()) {
                QFile dbusResult(reply.value());
                dbusResult.remove();
            }
            break;
        } default:
            ok = false;
            break;
        }
        if (!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
        }
        return res;
    }
#endif

    QRect geometry;
    for (QScreen *const screen : QGuiApplication::screens()) {
        geometry = geometry.united(screen->geometry());
    }

    QPixmap p(QApplication::primaryScreen()->grabWindow(
                  QApplication::desktop()->winId(),
                  geometry.x(),
                  geometry.y(),
                  geometry.width(),
                  geometry.height())
              );
    auto screenNumber = QApplication::desktop()->screenNumber();
    QScreen *screen = QApplication::screens()[screenNumber];
    p.setDevicePixelRatio(screen->devicePixelRatio());

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (includeCursor) {
        paintCursorOnPixmap(p, geometry);
    }
#endif
    return p;
}

QPixmap ScreenGrabber::grabScreen(int screenNumber, bool &ok, bool includeCursor) {
    // 勿用 desktop()->screen(n)->grab()（QWidget::grab）。
    // 该路径对不可见 QDesktopScreenWidget 自绘，得到 QPalette::Window 灰图，并非读屏。
    // 灰图根因与弃用路径：f54538ee / PR #1387；详见 notes/06e-kiran-flameshot缺陷.md。
    // 勿在整桌 pixmap 上用逻辑 screenGeometry 做 copy：HiDPI 下 pixmap 常为设备像素，
    // 逻辑矩形裁剪会错位（P1）。X11 对齐上游 v12 / PR #2076：对该屏 grabWindow。
    // Wayland 仍整桌抓取后再裁（单屏 grabWindow 不可靠）。
    // window=0 时 x/y 相对该 QScreen，勿传全局 screenGeometry 的 x/y（副屏会错屏/黑边）。

    if (screenNumber < 0 || screenNumber >= QGuiApplication::screens().size()) {
        ok = false;
        return QPixmap();
    }

    QScreen *screen = QGuiApplication::screens()[screenNumber];
    QRect geometry = QApplication::desktop()->screenGeometry(screenNumber);
    if (geometry.isEmpty()) {
        ok = false;
        return QPixmap();
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_info.waylandDectected()) {
        QPixmap p = grabEntireDesktop(ok, includeCursor);
        if (!ok) {
            return QPixmap();
        }
        // 光标已在 grabEntireDesktop 内处理；裁剪后勿再 paint
        return p.copy(geometry);
    }
#endif

    // X11 / 非 Wayland：单屏读屏（禁止 QWidget::grab）；grabWindow(0) 用屏内原点
    ok = true;
    QPixmap p = screen->grabWindow(0);
    if (p.isNull()) {
        ok = false;
        return QPixmap();
    }
    p.setDevicePixelRatio(screen->devicePixelRatio());

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (includeCursor) {
        // 在该屏逻辑几何上合成，勿对已 crop 结果二次整桌合成
        paintCursorOnPixmap(p, geometry);
    }
#endif
    return p;
}

QPixmap ScreenGrabber::grabActiveWindow(bool &ok, bool includeCursor) {
    // X11 only；root/desktop()->winId() + 全局逻辑坐标；禁止 grabWindow(0)
    ok = false;
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (!QX11Info::isPlatformX11()) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }

    Display *dpy = QX11Info::display();
    if (!dpy) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }

    Window root = static_cast<Window>(QX11Info::appRootWindow());
    Window active = readNetActiveWindow(dpy, root);
    // 没有可截取的激活窗口（无 active、桌面/DOCK、隐藏或不可见）时退化为整屏，
    // 避免用户点了「截取激活窗口」却只得到一句失败提示
    if (active == None || !isValidCaptureTarget(dpy, active)) {
        SystemNotification().sendMessage(
                    tr("No active window, capturing the entire desktop"));
        return grabEntireDesktop(ok, includeCursor);
    }

    Window rootRet = None;
    int geomX = 0, geomY = 0;
    unsigned int cw = 0, ch = 0, border = 0, depth = 0;
    if (!XGetGeometry(dpy, active, &rootRet, &geomX, &geomY, &cw, &ch, &border, &depth)) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }
    Q_UNUSED(rootRet);
    Q_UNUSED(geomX);
    Q_UNUSED(geomY);
    Q_UNUSED(border);
    Q_UNUSED(depth);

    int clientRootX = 0, clientRootY = 0;
    Window childIgnored = None;
    if (!XTranslateCoordinates(dpy, active, root, 0, 0,
                               &clientRootX, &clientRootY, &childIgnored)) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }
    Q_UNUSED(childIgnored);

    int left = 0, right = 0, top = 0, bottom = 0;
    readFrameExtents(dpy, active, &left, &right, &top, &bottom);

    // 外框 = clientRoot − extents；宽高含左右上下边框。
    // XGetGeometry / Translate / FRAME_EXTENTS 均为 X11 原生像素。
    QRect frameNative(clientRootX - left,
                      clientRootY - top,
                      static_cast<int>(cw) + left + right,
                      static_cast<int>(ch) + top + bottom);

    // 与 grabEntireDesktop 同源：QScreen::geometry / grabWindow 用全局逻辑坐标。
    // QT_SCALE_FACTOR≠1 时若把 native 当 logical 传入，会与虚拟桌面求交错位并二次放大裁错。
    // dpr 取自主屏（primary）：desktop()->screenNumber() 无参即主屏下标（非活动窗所在屏）。
    // 混合 DPI 副屏不在保证范围（已知不处理）。
    auto screenNumber = QApplication::desktop()->screenNumber();
    QScreen *screen = QApplication::screens()[screenNumber];
    qreal dpr = screen->devicePixelRatio();
    if (dpr <= 0.0) {
        dpr = 1.0;
    }
    QRect frame(qRound(frameNative.x() / dpr),
                qRound(frameNative.y() / dpr),
                qRound(frameNative.width() / dpr),
                qRound(frameNative.height() / dpr));

    QRect virtualDesktop;
    for (QScreen *const s : QGuiApplication::screens()) {
        virtualDesktop = virtualDesktop.united(s->geometry());
    }
    QRect clip = frame.intersected(virtualDesktop);
    if (clip.isEmpty()) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }

    // 固定 desktop()->winId()；全局逻辑坐标；禁止 grabWindow(0, 全局…)
    QPixmap p(QApplication::primaryScreen()->grabWindow(
                  QApplication::desktop()->winId(),
                  clip.x(),
                  clip.y(),
                  clip.width(),
                  clip.height()));
    if (p.isNull()) {
        SystemNotification().sendMessage(tr("Unable to capture active window"));
        return QPixmap();
    }

    p.setDevicePixelRatio(dpr);

    if (includeCursor) {
        // clip 已是逻辑坐标，与 QCursor::pos / paintCursorOnPixmap 约定一致
        paintCursorOnPixmap(p, clip);
    }
    ok = true;
    return p;
#else
    Q_UNUSED(includeCursor);
    SystemNotification().sendMessage(tr("Unable to capture active window"));
    return QPixmap();
#endif
}

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
void ScreenGrabber::paintCursorOnPixmap(QPixmap &pixmap, const QRect &captureGeometry)
{
    if (!QX11Info::isPlatformX11()) {
        return;
    }
    Display *dpy = QX11Info::display();
    if (!dpy) {
        return;
    }

    XFixesCursorImage *cursor = XFixesGetCursorImage(dpy);
    if (!cursor || cursor->width == 0 || cursor->height == 0) {
        if (cursor) {
            XFree(cursor);
        }
        return;
    }

    // XFixes GetCursorImage：低 32 位为已预乘 ARGB，勿再次 × alpha
    QImage cursorImg(cursor->width, cursor->height, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < cursor->height; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(cursorImg.scanLine(y));
        for (int x = 0; x < cursor->width; ++x) {
            unsigned long pixel = cursor->pixels[y * cursor->width + x];
            quint8 a = (pixel >> 24) & 0xff;
            quint8 r = (pixel >> 16) & 0xff;
            quint8 g = (pixel >> 8) & 0xff;
            quint8 b = pixel & 0xff;
            line[x] = qRgba(r, g, b, a);
        }
    }

    qreal dpr = pixmap.devicePixelRatio();
    if (dpr <= 0.0) {
        dpr = 1.0;
    }

    // 位置用 QCursor::pos（逻辑坐标，与 captureGeometry 一致）；
    // XFixes 图像为原生像素，设 DPR 后由 QPainter 按逻辑坐标绘制，避免位置×dpr 再 scaled 双重放大
    QPoint localLogical = QCursor::pos() - captureGeometry.topLeft();
    QPoint hotLogical(qRound(cursor->xhot / dpr), qRound(cursor->yhot / dpr));

    QPixmap cursorPixmap = QPixmap::fromImage(cursorImg);
    cursorPixmap.setDevicePixelRatio(dpr);

    QPainter painter(&pixmap);
    painter.drawPixmap(localLogical - hotLogical, cursorPixmap);

    XFree(cursor);
}
#endif
