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

// CLI 模式入口：解析命令行并把请求转交给常驻实例，返回进程退出码。
//
// 无参启动走 runDaemonApplication()，有参启动走本函数，两者共用同一个可执行文件。
// 历史行为：解析失败、checker 失败、默认目录解析失败都返回 0，错误只写 stdout。
int runCliApplication(int argc, char *argv[]);
