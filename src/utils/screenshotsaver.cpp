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

#include "screenshotsaver.h"
#include "src/utils/systemnotification.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QImageWriter>
#include <QDebug>

ScreenshotSaver::ScreenshotSaver() {
}

void ScreenshotSaver::saveToClipboard(const QPixmap &capture) {
    SystemNotification().sendMessage(
                QObject::tr("Capture saved to clipboard"));
    QApplication::clipboard()->setPixmap(capture);
}

bool ScreenshotSaver::saveToFilesystem(const QPixmap &capture,
                                       const QString &path,const QString &picture_format)
{
    QString completePath = FileNameHandler().generateAbsolutePath(path);

    //completePath += QLatin1String(".png");
    if(picture_format == QLatin1String("png"))
        completePath += QLatin1String(".png");
    else if(picture_format == QLatin1String("bmp"))
        completePath += QLatin1String(".bmp");
    else if(picture_format == QLatin1String("jpg"))
        completePath += QLatin1String(".jpg");

    bool ok = capture.save(completePath);
    QString saveMessage;
    QString notificationPath = completePath;

    if (ok) {
        ConfigHandler().setSavePath(path);
        saveMessage = QObject::tr("Capture saved as ") + completePath;
    } else {
        saveMessage = QObject::tr("Error trying to save as ") + completePath;
        notificationPath = "";
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

bool ScreenshotSaver::saveToFilesystemGUI(const QPixmap &capture,const QString &picture_format) {
    bool ok = false;

    while (!ok) {
        QString savePath = QFileDialog::getSaveFileName(
                    nullptr,
                    QString(),
                    FileNameHandler().absoluteSavePath() + "." + picture_format,
                    QObject::tr("Portable Network Graphic file (PNG) (*.png);;BMP file (*.bmp);;JPEG file (*.jpg)"));
        if (savePath.isNull()) {
            break;
        }

	if (!savePath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) &&
	    !savePath.endsWith(QLatin1String(".bmp"), Qt::CaseInsensitive) &&
	    !savePath.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive)) {
        if(picture_format == QLatin1String("png"))
            savePath += QLatin1String(".png");
        else if(picture_format == QLatin1String("bmp"))
            savePath += QLatin1String(".bmp");
        else if(picture_format == QLatin1String("jpg"))
            savePath += QLatin1String(".jpg");
    }



        ok = capture.save(savePath);

        if (ok) {
            QString pathNoFile = savePath.left(savePath.lastIndexOf(QLatin1String("/")));
            ConfigHandler().setSavePath(pathNoFile);
            QString msg = QObject::tr("Capture saved as ") + savePath;
            SystemNotification().sendMessage(msg, savePath);
        } else {
            QString msg = QObject::tr("Error trying to save as ") + savePath;
            QMessageBox saveErrBox(
                        QMessageBox::Warning,
                        QObject::tr("Save Error"),
                        msg);
            saveErrBox.setWindowIcon(QIcon(":img/app/kiran128x128.png"));
            saveErrBox.exec();
        }
    }
    return ok;
}

bool ScreenshotSaver::saveTo(const QPixmap &capture,
                                       const QString &path)
{
    QString completePath = FileNameHandler().generateAbsolutePath(path);
    completePath += QLatin1String(".png");
    bool ok = capture.save(completePath);
    QString saveMessage;
    QString notificationPath = completePath;

    if (ok) {
        ConfigHandler().setSavePath(path);
        saveMessage = QObject::tr("Capture saved as ") + completePath;
    } else {
        saveMessage = QObject::tr("Error trying to save as ") + completePath;
        notificationPath = "";
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

bool ScreenshotSaver::saveToGUI(const QPixmap &capture) {
    bool ok = false;

    while (!ok) {
        QString savePath = QFileDialog::getSaveFileName(
                    nullptr,
                    QString(),
                    FileNameHandler().absoluteSavePath() + ".png",
                    QObject::tr("Portable Network Graphic file (PNG) (*.png);;BMP file (*.bmp);;JPEG file (*.jpg)"));

        if (savePath.isNull()) {
            break;
        }

    if (!savePath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) &&
        !savePath.endsWith(QLatin1String(".bmp"), Qt::CaseInsensitive) &&
        !savePath.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive)) {

        savePath += QLatin1String(".png");
    }

        ok = capture.save(savePath);

        if (ok) {
            QString pathNoFile = savePath.left(savePath.lastIndexOf(QLatin1String("/")));
            ConfigHandler().setSavePath(pathNoFile);
            QString msg = QObject::tr("Capture saved as ") + savePath;
            SystemNotification().sendMessage(msg, savePath);
        } else {
            QString msg = QObject::tr("Error trying to save as ") + savePath;
            QMessageBox saveErrBox(
                        QMessageBox::Warning,
                        QObject::tr("Save Error"),
                        msg);
            saveErrBox.setWindowIcon(QIcon(":img/app/kiran128x128.png"));
            saveErrBox.exec();
        }
    }
    return ok;
}
