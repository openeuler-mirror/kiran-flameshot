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

// 两个运行模式共用同一个可执行文件：
//   - 无参启动：runDaemonApplication()，常驻实例，注册 org.dharkael.Flameshot；
//   - 有参启动：runCliApplication()，一次性客户端，把请求转交给常驻实例。
// 这里只保留两个模式共用的初始化，且必须早于任何应用对象构造。

#include "src/cli/cli-application.h"
#include "src/core/daemon-application.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QList>
#include <QMetaType>
#include <QString>
#include <qt5-log-i.h>

int main(int argc, char *argv[]) {
    // 按钮配置的序列化在两种运行模式下都可能用到，必须在构造应用对象前注册。
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    // 版本号在构造任何应用对象之前写入：--version 的输出来自这里。
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    klog_qt5_init("", "kylinsec-session", "kiran-flameshot", "flameshot");

    const bool daemonMode = (argc == 1);
    KLOG_INFO() << "start kiran-flameshot" << APP_VERSION
                << "argc=" << argc
                << "mode=" << (daemonMode ? "daemon" : "cli");

    if (daemonMode) {
        return runDaemonApplication(argc, argv);
    }
    return runCliApplication(argc, argv);
}
