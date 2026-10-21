cmake_minimum_required(VERSION 3.6)

project(kiran-flameshot VERSION 2.7.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
include(CTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProjectVersion)
include(InstallLayout)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg DBus X11Extras)
find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
find_package(PkgConfig REQUIRED)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
pkg_check_modules(XFIXES REQUIRED IMPORTED_TARGET xfixes)

include(Translations)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt5 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# 运行时资源；可执行文件本身由 src/CMakeLists.txt 负责安装。
install(FILES docs/desktopEntry/package/kiran-flameshot.desktop
        DESTINATION ${KFS_INSTALL_APPLICATIONS})
install(FILES docs/appdata/kiran-flameshot.appdata.xml
        DESTINATION ${KFS_INSTALL_METAINFO})
install(FILES dbus/package/org.dharkael.Flameshot.service
        DESTINATION ${KFS_INSTALL_DBUS_SERVICES})
install(FILES dbus/org.dharkael.Flameshot.xml
        DESTINATION ${KFS_INSTALL_DBUS_INTERFACES})
install(FILES docs/bash-completion/kiran-flameshot
        DESTINATION ${KFS_INSTALL_BASH_COMPLETION})
install(FILES ${KFS_QM_FILES}
        DESTINATION ${KFS_INSTALL_TRANSLATIONS})
install(FILES img/hicolor/48x48/apps/kiran-flameshot.png
        DESTINATION ${KFS_INSTALL_HICOLOR}/48x48/apps)
install(FILES img/hicolor/128x128/apps/kiran-flameshot.png
        DESTINATION ${KFS_INSTALL_HICOLOR}/128x128/apps)
install(FILES img/hicolor/scalable/apps/kiran-flameshot.png
        DESTINATION ${KFS_INSTALL_HICOLOR}/scalable/apps)
