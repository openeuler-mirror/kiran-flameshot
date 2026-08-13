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

#pragma once

#include "src/utils/desktopinfo.h"
#include <QObject>

class ScreenGrabber : public QObject {
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject *parent = nullptr);
    QPixmap grabEntireDesktop(bool &ok, bool includeCursor = false);
    QPixmap grabScreen(int screenNumber, bool &ok, bool includeCursor = false);

private:
    DesktopInfo m_info;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    // 在设备像素坐标系下把光标合成到 pixmap 上
    // captureGeometry: 捕获区域在全局逻辑坐标系中的矩形
    void paintCursorOnPixmap(QPixmap &pixmap, const QRect &captureGeometry);
#endif
};
