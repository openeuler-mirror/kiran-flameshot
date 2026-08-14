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
#include <X11/extensions/Xfixes.h>
#endif

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent) {

}

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
