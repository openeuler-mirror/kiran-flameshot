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

// 常驻模式入口（无参启动）：单实例 + 界面翻译 + D-Bus 服务注册 + 事件循环。
//
// 单实例语义由 SingleApplication 提供，是跨 DISPLAY、跨会话总线的全局互斥；
// 有参启动走 runCliApplication()，两者共用同一个可执行文件。
int runDaemonApplication(int argc, char *argv[]);
