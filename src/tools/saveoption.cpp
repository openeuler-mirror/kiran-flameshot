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

#include "saveoption.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <QProcess>
#include <QMenu>

SaveOption::SaveOption(QObject *parent) : AbstractActionTool(parent) {

}

bool SaveOption::closeOnButtonPressed() const {
    return false;
}

bool SaveOption::isSelectable() const{
    return true;
}

QIcon SaveOption::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "content-save.svg");
    return QIcon();
}
QString SaveOption::name() const {
    return tr("Option");
}

QString SaveOption::nameID() {
    return QLatin1String("");
}

QString SaveOption::description() const {
    return tr("Save options");
}

QMenu *SaveOption::initMenu(){
    return NULL;
}

CaptureTool* SaveOption::copy(QObject *parent) {
    return new SaveOption(parent);
}

void SaveOption::pressed(const CaptureContext &context) {
    //emit requestAction(REQ_OPTION);
    //qDebug("111");
    /*
    if (context.savePath.isEmpty()) {
        //emit requestAction(REQ_HIDE_GUI);
        bool ok = ScreenshotSaver().saveToFilesystemGUI(
                    context.selectedScreenshotArea());
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    } else {
        bool ok = ScreenshotSaver().saveToFilesystem(
                    context.selectedScreenshotArea(), context.savePath);
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    }*/
}
