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

#include "src/cli/cli-arguments.h"

#include "src/utils/systemnotification.h"

#include <QColor>
#include <QDir>

// 参数名、描述文本、默认值与声明顺序都参与 --help 输出与解析行为，原样保留。
CliArguments::CliArguments()
    : guiArgument(QStringLiteral("gui"),
                  QStringLiteral("Start a manual capture in GUI mode.")),
      screenArgument(QStringLiteral("screen"),
                     QStringLiteral("Capture a single screen.")),
      fullArgument(QStringLiteral("full"),
                   QStringLiteral("Capture the entire desktop.")),
      windowArgument(QStringLiteral("window"),
                     QStringLiteral("Capture the active window.")),
      launcherArgument(QStringLiteral("launcher"),
                       QStringLiteral("Open the capture launcher.")),
      configArgument(QStringLiteral("config"),
                     QStringLiteral("Configure flameshot.")),

      pathOption({"p", "path"},
                 QStringLiteral("Path where the capture will be saved"),
                 QStringLiteral("path")),
      clipboardOption({"c", "clipboard"},
                      QStringLiteral("Save the capture to the clipboard")),
      delayOption({"d", "delay"},
                  QStringLiteral("Delay time in milliseconds"),
                  QStringLiteral("milliseconds")),
      filenameOption({"f", "filename"},
                     QStringLiteral("Set the filename pattern"),
                     QStringLiteral("pattern")),
      trayOption({"t", "trayicon"},
                 QStringLiteral("Enable or disable the trayicon"),
                 QStringLiteral("bool")),
      autostartOption({"a", "autostart"},
                      QStringLiteral("Enable or disable run at startup"),
                      QStringLiteral("bool")),
      showHelpOption({"s", "showhelp"},
                     QStringLiteral("Show the help message in the capture mode"),
                     QStringLiteral("bool")),
      mainColorOption({"m", "maincolor"},
                      QStringLiteral("Define the main UI color"),
                      QStringLiteral("color-code")),
      contrastColorOption({"k", "contrastcolor"},
                          QStringLiteral("Define the contrast UI color"),
                          QStringLiteral("color-code")),
      rawImageOption({"r", "raw"},
                     QStringLiteral("Print raw PNG capture")),
      screenNumberOption(
          {"n", "number"},
          QStringLiteral("Define the screen to capture,\ndefault: screen containing the cursor"),
          QStringLiteral("Screen number"),
          QStringLiteral("-1")),

      // help/version 由 parser 生成，这里只是占位，declareRelations() 会替换。
      helpOption(QString(), QString()),
      versionOption(QString(), QString()),
      m_command(Command::None)
{
    // 参数模型在构造时装配一次；parse() 只负责解析。
    declareParser();
    declareCheckers();
    declareRelations();
}

bool CliArguments::parse(const QStringList &args)
{
    if (!parser.parse(args)) {
        return false;
    }
    m_command = detectCommand();
    return true;
}

// parser 自身的描述与通用错误消息。
void CliArguments::declareParser()
{
    parser.setDescription(
                QStringLiteral("Powerful yet simple to use screenshot software."));
    parser.setGeneralErrorMessage(QStringLiteral("See 'kiran-flameshot --help'."));
}

// 每组 checker 与它的错误消息、受影响的 option 放在一起，避免来回对照。
void CliArguments::declareCheckers()
{
    // 主色与对比色共用同一校验：必须是不带透明度的合法颜色。
    auto colorChecker = [](const QString &colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    const QString colorErr = "Invalid color, "
                             "this flag supports the following formats:\n"
                             "- #RGB (each of R, G, and B is a single hex digit)\n"
                             "- #RRGGBB\n- #RRRGGGBBB\n"
                             "- #RRRRGGGGBBBB\n"
                             "- Named colors like 'blue' or 'red'\n"
                             "You may need to escape the '#' sign as in '\\#FFF'";
    mainColorOption.addChecker(colorChecker, colorErr);
    contrastColorOption.addChecker(colorChecker, colorErr);

    // 延迟与屏幕号都要求非负整数。
    auto numericChecker = [](const QString &value) -> bool {
        return value.toInt() >= 0;
    };
    delayOption.addChecker(
                numericChecker,
                QStringLiteral("Invalid delay, it must be higher than 0"));
    screenNumberOption.addChecker(
                numericChecker,
                QStringLiteral("Invalid screen number, it must be non negative"));

    // 路径必须是已存在的目录；失败时另发系统通知（既有行为）。
    const QString pathErr = QStringLiteral("Invalid path, it must be a real path in the system");
    auto pathChecker = [pathErr](const QString &pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
            SystemNotification().sendMessage(QObject::tr(pathErr.toLatin1().data()));
        }
        return res;
    };
    pathOption.addChecker(pathChecker, pathErr);

    // 布尔选项只接受字面量 true/false。
    const QString booleanErr = QStringLiteral("Invalid value, it must be defined as 'true' or 'false'");
    auto booleanChecker = [](const QString &value) -> bool {
        return value == QLatin1String("true") || value == QLatin1String("false");
    };
    trayOption.addChecker(booleanChecker, booleanErr);
    autostartOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);
}

// 参数关系。AddArgument 的顺序即 --help 里 Arguments 段的顺序，不可随意调整；
// help/version 的生成也必须排在 AddArgument 之后、AddOptions 之前。
void CliArguments::declareRelations()
{
    parser.AddArgument(guiArgument);
    parser.AddArgument(screenArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(windowArgument);
    parser.AddArgument(launcherArgument);
    parser.AddArgument(configArgument);

    helpOption = parser.addHelpOption();
    versionOption = parser.addVersionOption();

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
}

// 六个子参数互斥，正常只命中一个；判断顺序沿用历史分派优先级（launcher 优先），
// 与 --help 里 Arguments 的显示顺序无关。
CliArguments::Command CliArguments::detectCommand() const
{
    if (parser.isSet(launcherArgument)) {
        return Command::Launcher;
    }
    if (parser.isSet(guiArgument)) {
        return Command::Gui;
    }
    if (parser.isSet(fullArgument)) {
        return Command::Full;
    }
    if (parser.isSet(windowArgument)) {
        return Command::Window;
    }
    if (parser.isSet(screenArgument)) {
        return Command::Screen;
    }
    if (parser.isSet(configArgument)) {
        return Command::Config;
    }
    return Command::None;
}
