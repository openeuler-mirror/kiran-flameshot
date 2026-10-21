include(${PROJECT_SOURCE_DIR}/cmake/LegacySources.cmake)

add_subdirectory(third-party)
add_subdirectory(platform/kiran)

# 应用目标要链接 bundled 库，因此这些库必须先定义好。把检查放在这里，可以让缺失的
# 库目标在 configure 阶段直接报错，而不是变成链接期难以定位的失败。
foreach(required_target kfs-singleapplication kfs-color-widgets kfs-platform-kiran)
    if(NOT TARGET ${required_target})
        message(FATAL_ERROR "required target not defined: ${required_target}")
    endif()
endforeach()

# legacy 上传器沿用历史上的默认 client id，允许发行构建用缓存变量覆盖；
# 该上传功能待移除。
set(KIRAN_FLAMESHOT_IMGUR_CLIENT_ID "313baf0c7b4d3ff"
    CACHE STRING "Imgur client id used by the legacy uploader")

# D-Bus 客户端代理由接口 XML 生成：方法名与参数顺序因此获得编译期校验，CLI 端不再
# 手写 QDBusMessage。生成的 .cpp/.h/.moc 随应用目标一起编译（宏已处理 moc 依赖）。
qt5_add_dbus_interface(KFS_DBUS_PROXY_SRCS
    "${PROJECT_SOURCE_DIR}/dbus/org.dharkael.Flameshot.xml"
    org_dharkael_flameshot)

add_executable(kiran-flameshot
    ${KFS_APP_SOURCES}
    ${KFS_APP_MOC_HEADERS}
    ${KFS_DBUS_PROXY_SRCS}
    ../graphics.qrc)

target_include_directories(kiran-flameshot PRIVATE
    ${PROJECT_SOURCE_DIR}
    # 生成的 org_dharkael_flameshot.h 所在目录
    ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(kiran-flameshot PRIVATE
    APP_VERSION="${KIRAN_FLAMESHOT_VERSION}"
    APP_PREFIX="${CMAKE_INSTALL_PREFIX}"
    IMGUR_CLIENT_ID="${KIRAN_FLAMESHOT_IMGUR_CLIENT_ID}"
    WNCK_I_KNOW_THIS_IS_UNSTABLE)

# legacy 源码使用了设计上会逐步移除的 Qt 废弃 API；不要让这些告警淹没新增的告警。
target_compile_options(kiran-flameshot PRIVATE -Wno-deprecated-declarations)

target_link_libraries(kiran-flameshot PRIVATE
    kfs-singleapplication
    kfs-color-widgets
    kfs-platform-kiran
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::DBus
    Qt5::X11Extras
    PkgConfig::X11
    PkgConfig::XFIXES)

install(TARGETS kiran-flameshot RUNTIME DESTINATION ${KFS_INSTALL_BINDIR})
