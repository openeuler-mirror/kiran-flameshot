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

#include "src/core/daemon-application.h"

#include "singleapplication.h"
#include "src/core/controller.h"
#include "src/core/flameshotdbusadapter.h"
#include "src/utils/pathinfo.h"
#include "src/utils/systemnotification.h"

#include <QDBusConnection>
#include <QLibraryInfo>
#include <QTranslator>
#include <qt5-log-i.h>

namespace {

void installTranslations(SingleApplication &app, QTranslator &applicationTranslator,
                         QTranslator &qtTranslator)
{
    const QStringList trPaths = PathInfo::translationsPaths();
    KLOG_DEBUG() << "daemon: translation search paths" << trPaths;

    bool applicationTranslationLoaded = false;
    for (const QString &path : trPaths) {
        const bool match = applicationTranslator.load(
            QLocale(), QStringLiteral("Internationalization"),
            QStringLiteral("_"), path);
        if (match) {
            applicationTranslationLoaded = true;
            KLOG_INFO() << "daemon: application translation loaded from" << path;
            break;
        }
    }
    if (!applicationTranslationLoaded) {
        KLOG_WARNING() << "daemon: no application translation found"
                       << "paths=" << trPaths;
    }

    if (qtTranslator.load(
            QLocale::system(), "qt", "_",
            QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        KLOG_DEBUG() << "daemon: qt translation loaded";
    } else {
        KLOG_WARNING() << "daemon: qt translation not loaded from"
                       << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    }

    app.installTranslator(&applicationTranslator);
    app.installTranslator(&qtTranslator);
}

void registerDbusService(Controller *controller)
{
    new FlameshotDBusAdapter(controller);
    KLOG_DEBUG() << "daemon: dbus adapter created";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.isConnected()) {
        KLOG_WARNING() << "daemon: session bus is not connected";
        SystemNotification().sendMessage(
                    QObject::tr("Unable to connect via DBus"));
    }
    if (!dbus.registerObject(QStringLiteral("/"), controller)) {
        KLOG_ERROR() << "daemon: failed to register dbus object at /";
    }
    if (!dbus.registerService(QStringLiteral("org.dharkael.Flameshot"))) {
        KLOG_ERROR() << "daemon: failed to register dbus service"
                     << "org.dharkael.Flameshot";
    } else {
        KLOG_INFO() << "daemon: dbus service registered"
                    << "org.dharkael.Flameshot";
    }
}

}

int runDaemonApplication(int argc, char *argv[])
{
    SingleApplication app(argc, argv);

    KLOG_INFO() << "daemon: single instance state"
                << "primary=" << app.isPrimary()
                << "secondary=" << app.isSecondary();

    // installTranslator 只保存指针，翻译对象必须活到 app.exec() 返回。
    QTranslator applicationTranslator;
    QTranslator qtTranslator;
    installTranslations(app, applicationTranslator, qtTranslator);

    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    app.setApplicationName(QStringLiteral("kiran-flameshot"));
    app.setOrganizationName(QStringLiteral("Dharkael"));
    KLOG_DEBUG() << "daemon: application identity"
                 << app.applicationName() << app.organizationName();

    Controller *controller = Controller::getInstance();
    registerDbusService(controller);
    // 导出信号必须在 D-Bus 接口注册后连接，否则会阻塞到导出结束。
    controller->enableExports();
    KLOG_DEBUG() << "daemon: capture exports enabled, entering event loop";

    const int exitCode = app.exec();
    KLOG_INFO() << "daemon: event loop exited with code" << exitCode;
    return exitCode;
}
