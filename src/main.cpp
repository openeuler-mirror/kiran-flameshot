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

#include "src/core/controller.h"
#include "singleapplication.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include "src/cli/commandlineparser.h"
#include "src/utils/systemnotification.h"
#include "src/utils/pathinfo.h"
#include "src/core/capturerequest.h"
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include <QDebug>

#include "src/utils/dbusutils.h"
#include <QDBusMessage>
#include <QDBusConnection>

#include "src/core/flameshotdbusadapter.h"

// raw 模式兜底超时（毫秒）
static const int RAW_WAIT_TIMEOUT_MS = 2000;
static const int RAW_GUI_WAIT_TIMEOUT_MS = 1000 * 60 * 15; // 15 minutes

// 等待 raw 捕获结果：订阅 DBus 信号 + 超时兜底 + 进入事件循环
// 返回 app.exec() 的退出码（0=成功, 1=失败或连接断开）
static int waitForRawCapture(QCoreApplication &app,
                             DBusUtils &dbusUtils,
                             QDBusConnection &sessionBus,
                             uint id,
                             int delay,
                             int timeoutExtra)
{
    dbusUtils.connectPrintCapture(sessionBus, id);
    QTimer timer;
    timer.setInterval(delay + timeoutExtra);
    QObject::connect(&timer, &QTimer::timeout, &app,
                     &QCoreApplication::quit);
    timer.start();
    return app.exec();
}

#include <qt5-log-i.h>
int main(int argc, char *argv[]) {
    // required for the button serialization
    // TODO: change to QVector in v1.0
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    qApp->setApplicationVersion(static_cast<QString>(APP_VERSION));
    klog_qt5_init("", "kylinsec-session", "kiran-flameshot", "flameshot");

    // no arguments, just launch Flameshot
    if (argc == 1) {
        SingleApplication app(argc, argv);

        QTranslator translator, qtTranslator;
        QStringList trPaths = PathInfo::translationsPaths();

        for (const QString &path: trPaths) {
            bool match = translator.load(QLocale(),
                                    QStringLiteral("Internationalization"), QStringLiteral("_"),
                                    path);
            if (match) {
                break;
            }
        }

        qtTranslator.load(QLocale::system(), "qt", "_",
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));

        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
        app.setApplicationName(QStringLiteral("kiran-flameshot"));
        app.setOrganizationName(QStringLiteral("Dharkael"));

        auto c = Controller::getInstance();
        new FlameshotDBusAdapter(c);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.isConnected()) {
            SystemNotification().sendMessage(
                        QObject::tr("Unable to connect via DBus"));
        }
        dbus.registerObject(QStringLiteral("/"), c);
        dbus.registerService(QStringLiteral("org.dharkael.Flameshot"));
        // Exporting captures must be connected after the dbus interface
        // or the dbus signal gets blocked until we end the exports.
        c->enableExports();
        return app.exec();
    }

    /*--------------|
     * CLI parsing  |
     * ------------*/
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kiran-flameshot"));
    app.setOrganizationName(QStringLiteral("Dharkael"));
    app.setApplicationVersion(qApp->applicationVersion());
    CommandLineParser parser;
    // Add description
    parser.setDescription(
                QStringLiteral("Powerful yet simple to use screenshot software."));
    parser.setGeneralErrorMessage(QStringLiteral("See 'kiran-flameshot --help'."));
    // Arguments
    CommandArgument fullArgument(QStringLiteral("full"), QStringLiteral("Capture the entire desktop."));
    CommandArgument windowArgument(QStringLiteral("window"), QStringLiteral("Capture the active window."));
    CommandArgument launcherArgument(QStringLiteral("launcher"), QStringLiteral("Open the capture launcher."));
    CommandArgument guiArgument(QStringLiteral("gui"), QStringLiteral("Start a manual capture in GUI mode."));
    CommandArgument configArgument(QStringLiteral("config"), QStringLiteral("Configure flameshot."));
    CommandArgument screenArgument(QStringLiteral("screen"), QStringLiteral("Capture a single screen."));

    // OptionsA
    CommandOption pathOption(
                {"p", "path"},
                QStringLiteral("Path where the capture will be saved"),
                QStringLiteral("path"));
    CommandOption clipboardOption(
                {"c", "clipboard"},
                QStringLiteral("Save the capture to the clipboard"));
    CommandOption delayOption(
                {"d", "delay"},
                QStringLiteral("Delay time in milliseconds"),
                QStringLiteral("milliseconds"));
    CommandOption filenameOption(
                {"f", "filename"},
                QStringLiteral("Set the filename pattern"),
                QStringLiteral("pattern"));
    CommandOption trayOption(
                {"t", "trayicon"},
                QStringLiteral("Enable or disable the trayicon"),
                QStringLiteral("bool"));
    CommandOption autostartOption(
                {"a", "autostart"},
                QStringLiteral("Enable or disable run at startup"),
                QStringLiteral("bool"));
    CommandOption showHelpOption(
                {"s", "showhelp"},
                QStringLiteral("Show the help message in the capture mode"),
                QStringLiteral("bool"));
    CommandOption mainColorOption(
                {"m", "maincolor"},
                QStringLiteral("Define the main UI color"),
                QStringLiteral("color-code"));
    CommandOption contrastColorOption(
                {"k", "contrastcolor"},
                QStringLiteral("Define the contrast UI color"),
                QStringLiteral("color-code"));
    CommandOption rawImageOption(
                {"r", "raw"},
                QStringLiteral("Print raw PNG capture"));
    CommandOption screenNumberOption(
                {"n", "number"},
                QStringLiteral("Define the screen to capture,\ndefault: screen containing the cursor"),
                QStringLiteral("Screen number"), QStringLiteral("-1"));

    // Add checkers
    auto colorChecker = [](const QString &colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    QString colorErr = "Invalid color, "
                       "this flag supports the following formats:\n"
                       "- #RGB (each of R, G, and B is a single hex digit)\n"
                       "- #RRGGBB\n- #RRRGGGBBB\n"
                       "- #RRRRGGGGBBBB\n"
                       "- Named colors like 'blue' or 'red'\n"
                       "You may need to escape the '#' sign as in '\\#FFF'";

    const QString delayErr = QStringLiteral("Invalid delay, it must be higher than 0");
    const QString numberErr = QStringLiteral("Invalid screen number, it must be non negative");
    auto numericChecker = [](const QString &delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };

    const QString pathErr = QStringLiteral("Invalid path, it must be a real path in the system");
    auto pathChecker = [pathErr](const QString &pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
            SystemNotification().sendMessage(QObject::tr(pathErr.toLatin1().data()));
        }
        return res;
    };

    const QString booleanErr = QStringLiteral("Invalid value, it must be defined as 'true' or 'false'");
    auto booleanChecker = [](const QString &value) -> bool {
        return value == QLatin1String("true") || value == QLatin1String("false");
    };

    contrastColorOption.addChecker(colorChecker, colorErr);
    mainColorOption.addChecker(colorChecker, colorErr);
    delayOption.addChecker(numericChecker, delayErr);
    pathOption.addChecker(pathChecker, pathErr);
    trayOption.addChecker(booleanChecker, booleanErr);
    autostartOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);
    screenNumberOption.addChecker(numericChecker, numberErr);

    // Relationships
    parser.AddArgument(guiArgument);
    parser.AddArgument(screenArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(windowArgument);
    parser.AddArgument(launcherArgument);
    parser.AddArgument(configArgument);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.AddOptions({ pathOption, delayOption, rawImageOption }, guiArgument);
    parser.AddOptions({ screenNumberOption, clipboardOption, pathOption,
                        delayOption, rawImageOption },
                      screenArgument);
    parser.AddOptions({ pathOption, clipboardOption, delayOption, rawImageOption },
                      fullArgument);
    parser.AddOptions({ pathOption, clipboardOption, delayOption, rawImageOption },
                      windowArgument);
    parser.AddOptions({ autostartOption, filenameOption, trayOption,
                        showHelpOption, mainColorOption, contrastColorOption },
                      configArgument);
    // Parse
    if (!parser.parse(app.arguments())) {
        goto finish;
    }

    // PROCESS DATA
    //--------------
    if (parser.isSet(helpOption) || parser.isSet(versionOption)) {
    }
    else if (parser.isSet(launcherArgument)) { // LAUNCHER
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                           QStringLiteral("/"), QLatin1String(""), QStringLiteral("openLauncher"));
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.isConnected()) {
            SystemNotification().sendMessage(
                        QObject::tr("Unable to connect via DBus"));
        }
        sessionBus.call(m);
    }
    else if (parser.isSet(guiArgument)) { // GUI
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool isRaw = parser.isSet(rawImageOption);
        DBusUtils dbusUtils;
        CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, pathValue);
        uint id = req.id();

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                           QStringLiteral("/"), QLatin1String(""), QStringLiteral("graphicCapture"));
        m << pathValue << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            return waitForRawCapture(app, dbusUtils, sessionBus, id, delay,
                                     RAW_GUI_WAIT_TIMEOUT_MS);
        }
    }
    else if (parser.isSet(fullArgument)) { // FULL
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // 原先无 -p/-c/-r 直接报错；快捷键常发裸 full，需放宽为默认目录落盘
        // 无 -p/-c/-r：resolve 默认目录；仅 -c/-r：不强制落盘；有 -p：用该路径
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            pathValue = FileNameHandler().resolveDefaultSaveDirectory();
            if (pathValue.isEmpty()) {
                QTextStream out(stdout);
                out << "Failed to resolve default save directory\n";
                SystemNotification().sendMessage(
                            QObject::tr("Failed to resolve default save directory"));
                goto finish;
            }
        }

        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue);
        if (toClipboard) {
            req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
        }
        if (!pathValue.isEmpty()) {
            req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
        }
        uint id = req.id();
        DBusUtils dbusUtils;

        // path, toClipboard, delay, isRaw, id
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("fullScreen"));
        m << pathValue << toClipboard << delay << isRaw << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            return waitForRawCapture(app, dbusUtils, sessionBus, id, delay,
                                     RAW_WAIT_TIMEOUT_MS);
        }
    }
    else if (parser.isSet(windowArgument)) { // WINDOW（激活窗；落盘对称 full）
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // 与 full 对称：无 -p/-c/-r 时 resolve 默认目录落盘
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            pathValue = FileNameHandler().resolveDefaultSaveDirectory();
            if (pathValue.isEmpty()) {
                QTextStream out(stdout);
                out << "Failed to resolve default save directory\n";
                SystemNotification().sendMessage(
                            QObject::tr("Failed to resolve default save directory"));
                goto finish;
            }
        }

        CaptureRequest req(CaptureRequest::WINDOW_MODE, delay, pathValue);
        if (toClipboard) {
            req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
        }
        if (!pathValue.isEmpty()) {
            req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
        }
        uint id = req.id();
        DBusUtils dbusUtils;

        // path, toClipboard, delay, isRaw, id；光标只读全局配置
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("captureWindow"));
        m << pathValue << toClipboard << delay << isRaw << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            return waitForRawCapture(app, dbusUtils, sessionBus, id, delay,
                                     RAW_WAIT_TIMEOUT_MS);
        }
    }
    else if (parser.isSet(screenArgument)) { // SCREEN
        QString numberStr = parser.value(screenNumberOption);
        int number = numberStr.startsWith(QLatin1String("-")) ? -1 : numberStr.toInt();
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // 与 full 对称：放宽门禁，无 -p/-c/-r 时 resolve 默认目录落盘
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            pathValue = FileNameHandler().resolveDefaultSaveDirectory();
            if (pathValue.isEmpty()) {
                QTextStream out(stdout);
                out << "Failed to resolve default save directory\n";
                SystemNotification().sendMessage(
                            QObject::tr("Failed to resolve default save directory"));
                goto finish;
            }
        }

        CaptureRequest req(CaptureRequest::SCREEN_MODE,
                           delay, pathValue, number);
        if (toClipboard) {
            req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
        }
        if (!pathValue.isEmpty()) {
            req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
        }
        uint id = req.id();
        DBusUtils dbusUtils;

        // number, path, toClipboard, delay, isRaw, id
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("captureScreen"));
        m << number << pathValue << toClipboard << delay << isRaw << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            return waitForRawCapture(app, dbusUtils, sessionBus, id, delay,
                                     RAW_WAIT_TIMEOUT_MS);
        }
    }
    else if (parser.isSet(configArgument)) { // CONFIG
        bool autostart = parser.isSet(autostartOption);
        bool filename = parser.isSet(filenameOption);
        bool tray = parser.isSet(trayOption);
        bool help = parser.isSet(showHelpOption);
        bool mainColor = parser.isSet(mainColorOption);
        bool contrastColor = parser.isSet(contrastColorOption);
        bool someFlagSet = (filename || tray || help ||
                            mainColor || contrastColor);
        ConfigHandler config;
        if (autostart) {
            QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("autostartEnabled"));
            if (parser.value(autostartOption) == QLatin1String("false")) {
                m << false;
            } else if (parser.value(autostartOption) == QLatin1String("true")) {
                m << true;
            }
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                            QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
        if (filename) {
            QString newFilename(parser.value(filenameOption));
            config.setFilenamePattern(newFilename);
            FileNameHandler fh;
            QTextStream(stdout)
                    << QStringLiteral("The new pattern is '%1'\n"
                                      "Parsed pattern example: %2\n").arg(newFilename)
                       .arg(fh.parsedPattern());
        }
        if (tray) {
            QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("trayIconEnabled"));
            if (parser.value(trayOption) == QLatin1String("false")) {
                m << false;
            } else if (parser.value(trayOption) == QLatin1String("true")) {
                m << true;
            }
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                            QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
        if (help) {
            if (parser.value(showHelpOption) == QLatin1String("false")) {
                config.setShowHelp(false);
            } else if (parser.value(showHelpOption) == QLatin1String("true")) {
                config.setShowHelp(true);
            }
        }
        if (mainColor) {
            QString colorCode = parser.value(mainColorOption);
            QColor parsedColor(colorCode);
            config.setUIMainColor(parsedColor);
        }
        if (contrastColor) {
            QString colorCode = parser.value(contrastColorOption);
            QColor parsedColor(colorCode);
            config.setUIContrastColor(parsedColor);
        }

        // Open gui when no options
        if (!someFlagSet) {
            QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.Flameshot"),
                                               QStringLiteral("/"), QLatin1String(""), QStringLiteral("openConfig"));
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                            QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
    }
finish:
    return 0;
}
