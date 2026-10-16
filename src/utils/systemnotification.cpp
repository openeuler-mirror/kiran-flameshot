#include "systemnotification.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QUrl>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>

SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}

void SystemNotification::sendMessage(const QString &text, const QString &savePath) {
    sendMessage(text, tr("Flameshot Info"), savePath);
}

void SystemNotification::sendMessage(
        const QString &text,
        const QString &title,
        const QString &savePath,
        const int timeout)
{
    if(!ConfigHandler().desktopNotificationValue()) {
        return;
    }

    QList<QVariant> args;
    QVariantMap hintsMap;
    if (!savePath.isEmpty()) {
        QUrl fullPath = QUrl::fromLocalFile(savePath);
        // allows the notification to be dragged and dropped
        hintsMap[QStringLiteral("x-kde-urls")] = QStringList({fullPath.toString()});
    }
    args << (qAppName())                 //appname
         << static_cast<unsigned int>(0) //id
         << "kiran"                  //icon
         << title                        //summary
         << text                         //body
         << QStringList()                //actions
         << hintsMap                     //hints
         << timeout;                     //timeout
    m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
}
