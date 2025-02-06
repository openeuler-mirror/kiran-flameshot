#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------

win32:LIBS += -luser32 -lshell32
QMAKE_CXXFLAGS  += -Wno-deprecated-declarations

TAG_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
isEmpty(TAG_VERSION){
    TAG_VERSION = v0.6.0
}
DEFINES += APP_VERSION=\\\"$$TAG_VERSION\\\"
DEFINES += WNCK_I_KNOW_THIS_IS_UNSTABLE

CONFIG += console

QT  += core gui widgets network svg
QT  += x11extras

unix:!macx {
    QT  += dbus
}

CONFIG += c++11 link_pkgconfig

PKGCONFIG += x11
PKGCONFIG += klog-qt5
PKGCONFIG += kiran-integration-theme

#CONFIG += packaging   # Enables "make install" for packaging paths
#LIBS += /usr/lib64/glibmm-2.4

INCLUDEPATH += /usr/src/debug/kiran-menu-2.2.0+alpha3-2.ky3.x86_64/lib
INCLUDEPATH += /usr/include/gdkmm-3.0
INCLUDEPATH += /usr/include/giomm-2.4
INCLUDEPATH += /usr/include/libwnck-3.0
INCLUDEPATH += /usr/include/sigc++-2.0
INCLUDEPATH += /usr/include/glibmm-2.4
INCLUDEPATH += /usr/include/glib-2.0
INCLUDEPATH += /usr/include/gtk-3.0/
INCLUDEPATH += /usr/include/gtkmm-3.0
INCLUDEPATH += /usr/include/cairo
INCLUDEPATH += /usr/include/pango-1.0
INCLUDEPATH += /usr/include/gdk-pixbuf-2.0/
INCLUDEPATH += /usr/include/cairomm-1.0
INCLUDEPATH += /usr/include/freetype2
INCLUDEPATH += /usr/include/atk-1.0/
INCLUDEPATH += /usr/lib64/glibmm-2.4/include
INCLUDEPATH += /usr/lib64/glib-2.0/include
INCLUDEPATH += /usr/lib64/sigc++-2.0/include
INCLUDEPATH += /usr/lib64/gdkmm-3.0/include
INCLUDEPATH += /usr/lib64/pangomm-1.4/include
INCLUDEPATH += /usr/lib64/giomm-2.4/include
INCLUDEPATH += /usr/lib64/cairomm-1.0/include

TARGET = kiran-flameshot

TEMPLATE = app

win32:RC_ICONS += img/app/flameshot.ico

#release: DESTDIR = build/release
#debug:   DESTDIR = build/debug

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

TRANSLATIONS = translations/Internationalization_es.ts \
    translations/Internationalization_ca.ts \
    translations/Internationalization_ru.ts \
    translations/Internationalization_zh_CN.ts \
    translations/Internationalization_zh_TW.ts \
    translations/Internationalization_tr.ts \
    translations/Internationalization_ka.ts \
    translations/Internationalization_fr.ts \
    translations/Internationalization_pl.ts \
    translations/Internationalization_ja.ts \
    translations/Internationalization_pt_br.ts \
    translations/Internationalization_sr.ts \
    translations/Internationalization_uk.ts \
    translations/Internationalization_de_DE.ts \
    translations/Internationalization_sk.ts \
    translations/Internationalization_bo_CN.ts \
    translations/Internationalization_kk_KG.ts \
    translations/Internationalization_kk_KZ.ts \
    translations/Internationalization_mn_MN.ts \
    translations/Internationalization_ug_CN.ts \

# Generate translations in build
TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

DEFINES += QT_DEPRECATED_WARNINGS

include(src/third-party/singleapplication/singleapplication.pri)
include(src/third-party/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

SOURCES += src/main.cpp \
    src/widgets/capture/buttonhandler.cpp \
    src/widgets/infowindow.cpp \
    src/config/configwindow.cpp \
    src/widgets/capture/capturewidget.cpp \
    src/widgets/capture/colorpicker.cpp \
    src/config/buttonlistview.cpp \
    src/config/uicoloreditor.cpp \
    src/config/geneneralconf.cpp \
    src/core/controller.cpp \
    src/config/clickablelabel.cpp \
    src/config/filenameeditor.cpp \
    src/config/strftimechooserwidget.cpp \
    src/widgets/capture/capturebutton.cpp \
    src/tools/pencil/penciltool.cpp \
    src/tools/undo/undotool.cpp \
    src/tools/redo/redotool.cpp \
    src/tools/arrow/arrowtool.cpp \
    src/tools/circle/circletool.cpp \
    src/tools/copy/copytool.cpp \
    src/tools/exit/exittool.cpp \
    src/tools/imgur/imguruploadertool.cpp \
    src/tools/line/linetool.cpp \
    src/tools/marker/markertool.cpp \
    src/tools/move/movetool.cpp \
    src/tools/rectangle/rectangletool.cpp \
    src/tools/save/savetool.cpp \
    src/tools/selection/selectiontool.cpp \
    src/tools/sizeindicator/sizeindicatortool.cpp \
    src/tools/toolfactory.cpp \
    src/utils/filenamehandler.cpp \
    src/utils/screengrabber.cpp \
    src/utils/confighandler.cpp \
    src/utils/systemnotification.cpp \
    src/cli/commandlineparser.cpp \
    src/cli/commandoption.cpp \
    src/cli/commandargument.cpp \
    src/utils/screenshotsaver.cpp \
    src/tools/imgur/imguruploader.cpp \
    src/widgets/loadspinner.cpp \
    src/widgets/imagelabel.cpp \
    src/widgets/notificationwidget.cpp \
    src/widgets/capture/notifierbox.cpp \
    src/utils/desktopinfo.cpp \
    src/tools/launcher/applauncherwidget.cpp \
    src/utils/desktopfileparse.cpp \
    src/tools/launcher/launcheritemdelegate.cpp \
    src/tools/blur/blurtool.cpp \
    src/tools/pin/pintool.cpp \
    src/tools/launcher/terminallauncher.cpp \
    src/config/visualseditor.cpp \
    src/config/extendedslider.cpp \
    src/tools/launcher/openwithprogram.cpp \
    src/tools/launcher/applaunchertool.cpp \
    src/utils/pathinfo.cpp \
    src/utils/colorutils.cpp \
    src/tools/capturecontext.cpp \
    src/widgets/capture/modificationcommand.cpp \
    src/tools/abstractpathtool.cpp \
    src/tools/abstracttwopointtool.cpp \
    src/tools/abstractactiontool.cpp \
    src/utils/globalvalues.cpp \
    src/widgets/panel/utilitypanel.cpp \
    src/widgets/capture/hovereventfilter.cpp \
    src/widgets/capture/selectionwidget.cpp \
    src/tools/pin/pinwidget.cpp \
    src/widgets/capturelauncher.cpp \
    src/tools/text/texttool.cpp \
    src/tools/text/textwidget.cpp \
    src/core/capturerequest.cpp \
    src/tools/text/textconfig.cpp \
    src/widgets/panel/sidepanelwidget.cpp \
    src/widgets/capture/zoomIndicator.cpp \
    src/utils/screenutils.cpp \
    src/utils/baseutils.cpp \
    src/widgets/capture/toptips.cpp \
    src/widgets/panel/buttonpanel.cpp \
    src/widgets/capture/menucontroller.cpp \
    src/widgets/capture/savetips.cpp \
    src/tools/saveoption.cpp \
    src/utils/win_manager.cpp \
    src/widgets/capture/toowidget.cpp \
    src/widgets/capture/thicknesswidget.cpp \
    src/widgets/capture/formtip.cpp

HEADERS  += src/widgets/capture/buttonhandler.h \
    src/widgets/infowindow.h \
    src/config/configwindow.h \
    src/widgets/capture/capturewidget.h \
    src/widgets/capture/colorpicker.h \
    src/config/buttonlistview.h \
    src/config/uicoloreditor.h \
    src/config/geneneralconf.h \
    src/config/clickablelabel.h \
    src/config/filenameeditor.h \
    src/utils/filenamehandler.h \
    src/config/strftimechooserwidget.h \
    src/utils/screengrabber.h \
    src/tools/capturetool.h \
    src/widgets/capture/capturebutton.h \
    src/tools/pencil/penciltool.h \
    src/tools/undo/undotool.h \
    src/tools/redo/redotool.h \
    src/tools/arrow/arrowtool.h \
    src/tools/circle/circletool.h \
    src/tools/copy/copytool.h \
    src/tools/exit/exittool.h \
    src/tools/imgur/imguruploadertool.h \
    src/tools/line/linetool.h \
    src/tools/marker/markertool.h \
    src/tools/move/movetool.h \
    src/tools/rectangle/rectangletool.h \
    src/tools/save/savetool.h \
    src/tools/selection/selectiontool.h \
    src/tools/sizeindicator/sizeindicatortool.h \
    src/tools/toolfactory.h \
    src/utils/confighandler.h \
    src/core/controller.h \
    src/utils/systemnotification.h \
    src/cli/commandlineparser.h \
    src/cli/commandoption.h \
    src/cli/commandargument.h \
    src/utils/screenshotsaver.h \
    src/tools/imgur/imguruploader.h \
    src/widgets/loadspinner.h \
    src/widgets/imagelabel.h \
    src/widgets/notificationwidget.h \
    src/widgets/capture/notifierbox.h \
    src/utils/desktopinfo.h \
    src/tools/launcher/applauncherwidget.h \
    src/utils/desktopfileparse.h \
    src/tools/launcher/launcheritemdelegate.h \
    src/tools/blur/blurtool.h \
    src/tools/pin/pintool.h \
    src/tools/launcher/terminallauncher.h \
    src/config/visualseditor.h \
    src/config/extendedslider.h \
    src/tools/launcher/openwithprogram.h \
    src/utils/pathinfo.h \
    src/tools/capturecontext.h \
    src/tools/launcher/applaunchertool.h \
    src/utils/globalvalues.h \
    src/utils/colorutils.h \
    src/widgets/capture/modificationcommand.h \
    src/tools/abstractpathtool.h \
    src/tools/abstracttwopointtool.h \
    src/tools/abstractactiontool.h \
    src/widgets/panel/utilitypanel.h \
    src/widgets/capture/hovereventfilter.h \
    src/widgets/capture/selectionwidget.h \
    src/tools/pin/pinwidget.h \
    src/widgets/capturelauncher.h \
    src/tools/text/texttool.h \
    src/tools/text/textwidget.h \
    src/core/capturerequest.h \
    src/tools/text/textconfig.h \
    src/widgets/panel/sidepanelwidget.h \
    src/widgets/capture/zoomIndicator.h \
    src/utils/screenutils.h \
    src/utils/baseutils.h \
    src/widgets/capture/toptips.h \
    src/widgets/panel/buttonpanel.h \
    src/utils/saveutils.h \
    src/widgets/capture/menucontroller.h \
    src/widgets/capture/savetips.h \
    src/tools/text/textbutton.h \
    src/tools/saveoption.h \
    src/tools/toolbutton.h \
    src/utils/win_manager.h \
    src/widgets/capture/toowidget.h \
    src/widgets/capture/thicknesswidget.h \
    src/widgets/capture/formtip.h

unix:!macx {
    SOURCES += src/core/flameshotdbusadapter.cpp \
        src/utils/dbusutils.cpp

    HEADERS  += src/core/flameshotdbusadapter.h \
        src/utils/dbusutils.h
}

win32 {
    SOURCES += src/core/globalshortcutfilter.cpp

    HEADERS  += src/core/globalshortcutfilter.h
}

RESOURCES += \
    graphics.qrc

# installs
unix:!macx {
    isEmpty(PREFIX) {
      packaging {
        PREFIX = /usr
      } else {
        PREFIX = /usr/local
      }
    }

    DEFINES += APP_PREFIX=\\\"$$PREFIX\\\"

    target.path = $${PREFIX}/bin/

    qmfile.path = $${PREFIX}/share/kiran-flameshot/translations/
    qmfile.files = $${TRANSLATIONS_FILES}

    dbus.path = $${PREFIX}/share/dbus-1/interfaces/
    dbus.files = dbus/org.dharkael.Flameshot.xml

    icon.path = $${PREFIX}/share/icons/hicolor/
    icon.files = img/hicolor/*

    completion.path = $${PREFIX}/share/bash-completion/completions/
    completion.files = docs/bash-completion/kiran-flameshot

    appdata.path = $${PREFIX}/share/metainfo/
    appdata.files = docs/appdata/kiran-flameshot.appdata.xml

    desktopentry.path = $${PREFIX}/share/applications
    desktopentry.files = docs/desktopEntry/package/kiran-flameshot.desktop

    servicedbus.path = $${PREFIX}/share/dbus-1/services/

    packaging {
        servicedbus.files = dbus/package/org.dharkael.Flameshot.service
    } else {
        servicedbus.files = dbus/make/org.dharkael.Flameshot.service
    }

    INSTALLS += target \
        icon \
        desktopentry \
        qmfile \
        servicedbus \
        dbus \
        completion \
        appdata
}

# Imgur API data
include(src/imgur.pri)

DISTFILES += \
    img/material/black/content-save.png \
    img/material/black/content-save.svg \
    img/action/text_normal@2x.png \
    img/action/text_normal.png \
    img/action/text_hover@2x.png \
    img/action/text_hover.png \
    img/action/text_checked@2x.png \
    img/action/text_checked.png \
    img/action/straightline_normal@2x.png \
    img/action/straightline_normal.png \
    img/action/straightline_hover@2x.png \
    img/action/straightline_hover.png \
    img/action/straightline_checked@2x.png \
    img/action/straightline_checked.png \
    img/action/straight_arrow_normal.png \
    img/action/straight_arrow_hover.png \
    img/action/straight_arrow_checked.png \
    img/action/sizetip.png \
    img/action/save_normal@2x.png \
    img/action/save_normal.png \
    img/action/save_hover@2x.png \
    img/action/save_hover.png \
    img/action/save_checked@2x.png \
    img/action/save_checked.png \
    img/action/rectangle_normal@2x.png \
    img/action/rectangle_normal.png \
    img/action/rectangle_hover@2x.png \
    img/action/rectangle_hover.png \
    img/action/rectangle_checked@2x.png \
    img/action/rectangle_checked.png \
    img/action/pencil_normal@2x.png \
    img/action/pencil_normal.png \
    img/action/pencil_hover@2x.png \
    img/action/pencil_hover.png \
    img/action/pencil_checked@2x.png \
    img/action/pencil_checked.png \
    img/action/oval_normal@2x.png \
    img/action/oval_normal.png \
    img/action/oval_hover@2x.png \
    img/action/oval_hover.png \
    img/action/oval_checked@2x.png \
    img/action/oval_checked.png \
    img/action/magnifier.png \
    img/action/close_normal@2x.png \
    img/action/close_normal.png \
    img/action/close_hover@2x.png \
    img/action/close_hover.png \
    img/action/close_checked@2x.png \
    img/action/close_checked.png \
    img/action/center_rect.png \
    img/action/arrow_normal@2x.png \
    img/action/arrow_normal.png \
    img/action/arrow_hover@2x.png \
    img/action/arrow_hover.png \
    img/action/arrow_checked@2x.png \
    img/action/arrow_checked.png \
    img/action/colors_normal.svg \
    img/action/colors_hover.svg \
    img/action/colors_checked.svg \
    img/menu_icons/undo-menu-press.svg \
    img/menu_icons/undo-menu-normal.svg \
    img/menu_icons/undo-menu-hover.svg \
    img/menu_icons/text-menu-press.svg \
    img/menu_icons/text-menu-norml.svg \
    img/menu_icons/text-menu-hover.svg \
    img/menu_icons/save-menu-press.svg \
    img/menu_icons/save-menu-norml.svg \
    img/menu_icons/save-menu-hover.svg \
    img/menu_icons/rectangle-menu-press.svg \
    img/menu_icons/rectangle-menu-norml.svg \
    img/menu_icons/rectangle-menu-hover.svg \
    img/menu_icons/line-menu-press.svg \
    img/menu_icons/line-menu-norml.svg \
    img/menu_icons/line-menu-hover.svg \
    img/menu_icons/exit-menu-press.svg \
    img/menu_icons/exit-menu-norml.svg \
    img/menu_icons/exit-menu-hover.svg \
    img/menu_icons/ellipse-menu-press.svg \
    img/menu_icons/ellipse-menu-norml.svg \
    img/menu_icons/ellipse-menu-hover.svg \
    img/menu_icons/arrow-menu-press.svg \
    img/menu_icons/arrow-menu-norml.svg \
    img/menu_icons/arrow-menu-hover.svg \
    qss/zoomindicator.qss \
    qss/subtoolbar.qss \
    qss/savetips.qss \
    docs/appdata/kiran-flameshot.appdata.xml \
    qss/colorbtn.qss \




