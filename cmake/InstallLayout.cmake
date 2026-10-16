# kiran-flameshot 运行时资源的安装目录。
#
# 所有路径都由 GNUInstallDirs 推导，发行版可以用 CMAKE_INSTALL_PREFIX 整体重定位；
# 这里不使用任何来自 RPM spec 的路径。
set(KFS_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
set(KFS_INSTALL_APPLICATIONS "${CMAKE_INSTALL_DATADIR}/applications")
set(KFS_INSTALL_METAINFO "${CMAKE_INSTALL_DATADIR}/metainfo")
set(KFS_INSTALL_DBUS_SERVICES "${CMAKE_INSTALL_DATADIR}/dbus-1/services")
set(KFS_INSTALL_DBUS_INTERFACES "${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces")
set(KFS_INSTALL_BASH_COMPLETION "${CMAKE_INSTALL_DATADIR}/bash-completion/completions")
set(KFS_INSTALL_HICOLOR "${CMAKE_INSTALL_DATADIR}/icons/hicolor")
set(KFS_INSTALL_TRANSLATIONS "${CMAKE_INSTALL_DATADIR}/kiran-flameshot/translations")
