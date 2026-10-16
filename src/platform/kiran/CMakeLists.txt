# Kiran 平台依赖，与可移植模块隔离。
#
# 2.7/2.8 使用 kiran-integration-theme；2.2 移植改用 kiran-style-helper。
pkg_check_modules(KIRAN_PLATFORM REQUIRED IMPORTED_TARGET
    klog-qt5
    kiran-integration-theme)

add_library(kfs-platform-kiran INTERFACE)
target_link_libraries(kfs-platform-kiran INTERFACE PkgConfig::KIRAN_PLATFORM)
