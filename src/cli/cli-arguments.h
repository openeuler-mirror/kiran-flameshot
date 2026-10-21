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

#include "src/cli/commandargument.h"
#include "src/cli/commandlineparser.h"
#include "src/cli/commandoption.h"

// CLI 模式的全部参数声明与解析结果。
//
// 声明内容集中在 cli-arguments.cpp：参数模型（本文件）与执行（cli-application.cpp）
// 分开，main 只负责选择运行模式。参数名、描述文本与声明顺序都参与 --help 输出，
// 不得改动。
class CliArguments {
public:
    CliArguments();

    // 解析失败时错误信息已由 parser 输出。
    bool parse(const QStringList &args);

    CommandLineParser parser;

    CommandArgument guiArgument;
    CommandArgument screenArgument;
    CommandArgument fullArgument;
    CommandArgument windowArgument;
    CommandArgument launcherArgument;
    CommandArgument configArgument;

    CommandOption pathOption;
    CommandOption clipboardOption;
    CommandOption delayOption;
    CommandOption filenameOption;
    CommandOption trayOption;
    CommandOption autostartOption;
    CommandOption showHelpOption;
    CommandOption mainColorOption;
    CommandOption contrastColorOption;
    CommandOption rawImageOption;
    CommandOption screenNumberOption;

    // 由 parser 生成，声明顺序上必须在其他参数之后。
    CommandOption helpOption;
    CommandOption versionOption;

    // 被选中的子命令。六个子参数互斥，解析后最多只有一个成立。
    enum class Command {
        None,
        Launcher,
        Gui,
        Full,
        Window,
        Screen,
        Config,
    };

    Command command() const { return m_command; }

private:
    // 参数模型装配：构造时执行一次，parse() 不再重复声明。
    void declareParser();
    void declareCheckers();
    void declareRelations();

    // 依据 parser 的解析结果判定当前子命令，顺序沿用历史分派优先级。
    Command detectCommand() const;

    Command m_command;
};
