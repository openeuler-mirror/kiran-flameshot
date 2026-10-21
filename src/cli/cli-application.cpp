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

#include "src/cli/cli-application.h"

#include "org_dharkael_flameshot.h"
#include "src/cli/cli-arguments.h"
#include "src/core/capturerequest.h"
#include "src/utils/confighandler.h"
#include "src/utils/dbusutils.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QTextStream>
#include <QTimer>
#include <qt5-log-i.h>

// raw 模式兜底超时（毫秒）
static const int RAW_WAIT_TIMEOUT_MS = 2000;
static const int RAW_GUI_WAIT_TIMEOUT_MS = 1000 * 60 * 15; // 15 minutes

namespace {

// 常驻实例注册的服务名与对象路径，与 dbus/org.dharkael.Flameshot.xml 保持一致。
const QString &daemonService()
{
    static const QString service = QStringLiteral("org.dharkael.Flameshot");
    return service;
}

const QString &daemonPath()
{
    static const QString path = QStringLiteral("/");
    return path;
}

// 与常驻实例的连接不可用时发出通知。既有行为：只通知，不改退出码。
bool notifyIfDisconnected(const QDBusConnection &bus)
{
    if (bus.isConnected()) {
        return true;
    }
    KLOG_WARNING() << "cli: session bus is not connected";
    SystemNotification().sendMessage(QObject::tr("Unable to connect via DBus"));
    return false;
}

// 等待 raw 捕获结果：订阅 DBus 信号 + 超时兜底 + 进入事件循环
// 返回 app.exec() 的退出码（0=成功, 1=失败或连接断开）
int waitForRawCapture(QCoreApplication &app,
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

// 日志用的模式名；仅用于输出，不参与任何逻辑判断。
const char *captureModeName(CaptureRequest::CaptureMode mode)
{
    switch (mode) {
    case CaptureRequest::GRAPHICAL_MODE:
        return "graphical";
    case CaptureRequest::FULLSCREEN_MODE:
        return "fullscreen";
    case CaptureRequest::WINDOW_MODE:
        return "window";
    case CaptureRequest::SCREEN_MODE:
        return "screen";
    }
    return "unknown";
}

void requestCapture(const QDBusConnection &bus,
                    CaptureRequest::CaptureMode mode,
                    int number,
                    const QString &path,
                    bool toClipboard,
                    int delay,
                    bool isRaw,
                    uint id)
{
    KLOG_DEBUG() << "cli: send capture request"
                 << "mode=" << captureModeName(mode)
                 << "id=" << id
                 << "number=" << number
                 << "path=" << path
                 << "clipboard=" << toClipboard
                 << "delay=" << delay
                 << "raw=" << isRaw;

    OrgDharkaelFlameshotInterface iface(daemonService(), daemonPath(), bus);
    switch (mode) {
    case CaptureRequest::GRAPHICAL_MODE:
        iface.graphicCapture(path, delay, id);
        break;
    case CaptureRequest::FULLSCREEN_MODE:
        iface.fullScreen(path, toClipboard, delay, isRaw, id);
        break;
    case CaptureRequest::WINDOW_MODE:
        // 光标是否入图只读全局配置，因此这里不传独立参数。
        iface.captureWindow(path, toClipboard, delay, isRaw, id);
        break;
    case CaptureRequest::SCREEN_MODE:
        iface.captureScreen(number, path, toClipboard, delay, isRaw, id);
        break;
    }
}

int runCapture(const CliArguments &arguments, QCoreApplication &app,
                CaptureRequest::CaptureMode mode)
{
    const CommandLineParser &parser = arguments.parser;
    QString pathValue = parser.value(arguments.pathOption);
    const int delay = parser.value(arguments.delayOption).toInt();
    const bool isRaw = parser.isSet(arguments.rawImageOption);
    const bool toClipboard = parser.isSet(arguments.clipboardOption);
    int screenNumber = -1;

    // -n 默认值是 "-1"（光标所在屏）。以 '-' 开头一律记为未指定，其余按非负整数解析。
    if (mode == CaptureRequest::SCREEN_MODE) {
        const QString numberStr = parser.value(arguments.screenNumberOption);
        screenNumber = numberStr.startsWith(QLatin1String("-")) ? -1 : numberStr.toInt();
        KLOG_DEBUG() << "cli: screen number argument" << numberStr << "resolved=" << screenNumber;
    }

    KLOG_DEBUG() << "cli: capture options"
                 << "mode=" << captureModeName(mode)
                 << "delay=" << delay
                 << "clipboard=" << toClipboard
                 << "raw=" << isRaw
                 << "path=" << pathValue;

    // 无 -p/-c/-r 时解析默认落盘目录（历史行为：快捷键常发裸 full）。
    // gui 不参与：裸 gui 由编辑器决定保存位置。
    if (mode != CaptureRequest::GRAPHICAL_MODE && !isRaw && !toClipboard &&
        pathValue.isEmpty()) {
        pathValue = FileNameHandler().resolveDefaultSaveDirectory();
        if (pathValue.isEmpty()) {
            KLOG_WARNING() << "cli: failed to resolve default save directory";
            QTextStream out(stdout);
            out << "Failed to resolve default save directory\n";
            SystemNotification().sendMessage(QObject::tr("Failed to resolve default save directory"));
            return 0;
        }
    }

    // 这份 CaptureRequest 只用于生成请求 id；保存意图通过 D-Bus 的 path/toClipboard
    // 等参数传给常驻实例，由 FlameshotDBusAdapter 按这些参数重建 CaptureRequest。
    CaptureRequest req(mode, delay, pathValue,
                       mode == CaptureRequest::SCREEN_MODE ? QVariant(screenNumber)
                                                           : QVariant());
    const uint id = req.id();

    // 会话总线不可用时发通知后退出。既有行为：进程退出码仍为 0。
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected()) {
        KLOG_WARNING() << "cli: session bus is not connected, skip capture request";
        DBusUtils connectionCheck;
        connectionCheck.checkDBusConnection(sessionBus);
        return 0;
    }

    // 抓取在常驻实例里执行；本进程只发对应的 D-Bus 方法。
    DBusUtils dbusUtils;
    requestCapture(sessionBus, mode, screenNumber, pathValue, toClipboard, delay, isRaw, id);

    // -r 要把 PNG 打到 stdout，必须等到 captureTaken/captureFailed。
    // gui 有人工选区，超时拉到 15 分钟；其余模式只留 delay 之外的 2 秒兜底。
    if (isRaw) {
        const int timeout = (mode == CaptureRequest::GRAPHICAL_MODE)
                                ? RAW_GUI_WAIT_TIMEOUT_MS : RAW_WAIT_TIMEOUT_MS;
        KLOG_INFO() << "cli: waiting for raw capture result"
                    << "id=" << id
                    << "delay=" << delay
                    << "timeout_ms=" << timeout;
        const int exitCode =
                waitForRawCapture(app, dbusUtils, sessionBus, id, delay, timeout);
        KLOG_INFO() << "cli: raw capture finished with code" << exitCode;
        return exitCode;
    }

    return 0;
}

int runLauncher()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (notifyIfDisconnected(sessionBus)) {
        OrgDharkaelFlameshotInterface iface(daemonService(), daemonPath(), sessionBus);
        iface.openLauncher();
    }
    return 0;
}

int runConfig(const CliArguments &arguments)
{
    const CommandLineParser &parser = arguments.parser;
    const bool autostart = parser.isSet(arguments.autostartOption);
    const bool filename = parser.isSet(arguments.filenameOption);
    const bool tray = parser.isSet(arguments.trayOption);
    const bool help = parser.isSet(arguments.showHelpOption);
    const bool mainColor = parser.isSet(arguments.mainColorOption);
    const bool contrastColor = parser.isSet(arguments.contrastColorOption);
    const bool someFlagSet = (filename || tray || help ||
                              mainColor || contrastColor);
    KLOG_INFO() << "cli: config request"
                << "autostart=" << autostart
                << "filename=" << filename
                << "tray=" << tray
                << "showhelp=" << help
                << "maincolor=" << mainColor
                << "contrastcolor=" << contrastColor;
    ConfigHandler config;

    if (autostart) {
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (notifyIfDisconnected(sessionBus)) {
            OrgDharkaelFlameshotInterface iface(daemonService(), daemonPath(), sessionBus);
            iface.autostartEnabled(
                        parser.value(arguments.autostartOption) != QLatin1String("false"));
        }
    }

    if (filename) {
        QString newFilename(parser.value(arguments.filenameOption));
        config.setFilenamePattern(newFilename);
        KLOG_INFO() << "cli: filename pattern updated to" << newFilename;
        FileNameHandler fh;
        QTextStream(stdout)
                << QStringLiteral("The new pattern is '%1'\n"
                                  "Parsed pattern example: %2\n").arg(newFilename)
                   .arg(fh.parsedPattern());
    }

    if (tray) {
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (notifyIfDisconnected(sessionBus)) {
            OrgDharkaelFlameshotInterface iface(daemonService(), daemonPath(), sessionBus);
            iface.trayIconEnabled(
                        parser.value(arguments.trayOption) != QLatin1String("false"));
        }
    }

    if (help) {
        if (parser.value(arguments.showHelpOption) == QLatin1String("false")) {
            config.setShowHelp(false);
            KLOG_INFO() << "cli: showhelp disabled";
        } else if (parser.value(arguments.showHelpOption) == QLatin1String("true")) {
            config.setShowHelp(true);
            KLOG_INFO() << "cli: showhelp enabled";
        }
    }

    if (mainColor) {
        QString colorCode = parser.value(arguments.mainColorOption);
        QColor parsedColor(colorCode);
        config.setUIMainColor(parsedColor);
        KLOG_INFO() << "cli: main color updated to" << colorCode;
    }

    if (contrastColor) {
        QString colorCode = parser.value(arguments.contrastColorOption);
        QColor parsedColor(colorCode);
        config.setUIContrastColor(parsedColor);
        KLOG_INFO() << "cli: contrast color updated to" << colorCode;
    }

    // Open gui when no options
    if (!someFlagSet) {
        KLOG_INFO() << "cli: open config window on the resident instance";
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (notifyIfDisconnected(sessionBus)) {
            OrgDharkaelFlameshotInterface iface(daemonService(), daemonPath(), sessionBus);
            iface.openConfig();
        }
    }

    return 0;
}

// 日志用的子命令名；仅用于输出，不参与任何逻辑判断。
const char *commandName(CliArguments::Command command)
{
    switch (command) {
    case CliArguments::Command::Launcher:
        return "launcher";
    case CliArguments::Command::Gui:
        return "gui";
    case CliArguments::Command::Full:
        return "full";
    case CliArguments::Command::Window:
        return "window";
    case CliArguments::Command::Screen:
        return "screen";
    case CliArguments::Command::Config:
        return "config";
    case CliArguments::Command::None:
        break;
    }
    return "none";
}

} // namespace

int runCliApplication(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kiran-flameshot"));
    app.setOrganizationName(QStringLiteral("Dharkael"));
    app.setApplicationVersion(qApp->applicationVersion());

    KLOG_INFO() << "cli: start kiran-flameshot"
                << app.applicationVersion()
                << "argc=" << argc
                << "arguments=" << app.arguments();

    CliArguments arguments;
    if (!arguments.parse(app.arguments())) {
        KLOG_WARNING() << "cli: argument parsing failed";
        return 0;
    }

    KLOG_DEBUG() << "cli: arguments parsed";

    if (arguments.parser.isSet(arguments.helpOption) ||
        arguments.parser.isSet(arguments.versionOption)) {
        KLOG_DEBUG() << "cli: help or version request served";
        return 0;
    }

    const CliArguments::Command command = arguments.command();
    if (command == CliArguments::Command::None) {
        KLOG_DEBUG() << "cli: nothing to do";
        return 0;
    }

    KLOG_INFO() << "cli: dispatch" << commandName(command);

    switch (command) {
    case CliArguments::Command::Launcher:
        return runLauncher();
    case CliArguments::Command::Gui:
        return runCapture(arguments, app, CaptureRequest::GRAPHICAL_MODE);
    case CliArguments::Command::Full:
        return runCapture(arguments, app, CaptureRequest::FULLSCREEN_MODE);
    case CliArguments::Command::Window:
        return runCapture(arguments, app, CaptureRequest::WINDOW_MODE);
    case CliArguments::Command::Screen:
        return runCapture(arguments, app, CaptureRequest::SCREEN_MODE);
    case CliArguments::Command::Config:
        return runConfig(arguments);
    case CliArguments::Command::None:
        break;
    }
    return 0;
}
