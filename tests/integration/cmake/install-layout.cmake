# 安装后运行时资源的断言检查。
#
# 两种模式：
#   staging 驱动：cmake -DKFS_BUILD_DIR=<build> -DSTAGE=<dir> -P install-layout.cmake
#                 先清空 STAGE，再安装进去，然后断言安装布局。
#   仅做断言：    cmake -DSTAGE=<已存在目录> -P install-layout.cmake
if(NOT DEFINED STAGE OR STAGE STREQUAL "")
    message(FATAL_ERROR "STAGE is required")
endif()

if(DEFINED KFS_BUILD_DIR AND NOT KFS_BUILD_DIR STREQUAL "")
    if(NOT IS_DIRECTORY "${KFS_BUILD_DIR}")
        message(FATAL_ERROR "missing build directory: ${KFS_BUILD_DIR}")
    endif()
    file(REMOVE_RECURSE "${STAGE}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env "DESTDIR=${STAGE}"
                ${CMAKE_COMMAND} --install "${KFS_BUILD_DIR}" --prefix /usr
        RESULT_VARIABLE install_rc
        OUTPUT_VARIABLE install_out
        ERROR_VARIABLE install_err)
    if(NOT install_rc EQUAL 0)
        message(FATAL_ERROR
            "staging install failed: rc=${install_rc}\n${install_out}\n${install_err}")
    endif()
endif()

set(required
    "${STAGE}/usr/bin/kiran-flameshot"
    "${STAGE}/usr/share/applications/kiran-flameshot.desktop"
    "${STAGE}/usr/share/metainfo/kiran-flameshot.appdata.xml"
    "${STAGE}/usr/share/dbus-1/services/org.dharkael.Flameshot.service"
    "${STAGE}/usr/share/dbus-1/interfaces/org.dharkael.Flameshot.xml"
    "${STAGE}/usr/share/bash-completion/completions/kiran-flameshot"
    "${STAGE}/usr/share/kiran-flameshot/translations/Internationalization_zh_CN.qm"
    "${STAGE}/usr/share/icons/hicolor/48x48/apps/kiran-flameshot.png"
    "${STAGE}/usr/share/icons/hicolor/128x128/apps/kiran-flameshot.png"
    "${STAGE}/usr/share/icons/hicolor/scalable/apps/kiran-flameshot.png")
foreach(path IN LISTS required)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "missing installed file: ${path}")
    endif()
endforeach()

# 运行时安装中不应出现调试源码或打包构建配方。
if(EXISTS "${STAGE}/usr/src" OR EXISTS "${STAGE}/usr/share/kiran-flameshot/rpm")
    message(FATAL_ERROR "forbidden packaging/debug content installed")
endif()

# D-Bus 激活文件与 desktop entry 引用的是绝对路径，必须指向本次安装实际提供的文件。
set(service_file "${STAGE}/usr/share/dbus-1/services/org.dharkael.Flameshot.service")
file(READ "${service_file}" service_body)
string(FIND "${service_body}" "Exec=/usr/bin/kiran-flameshot" service_pos)
if(service_pos EQUAL -1)
    message(FATAL_ERROR "dbus activation does not exec the installed binary: ${service_file}")
endif()

set(desktop_file "${STAGE}/usr/share/applications/kiran-flameshot.desktop")
file(READ "${desktop_file}" desktop_body)
string(REGEX MATCH "Icon=([^;\n]+)" desktop_icon_line "${desktop_body}")
set(desktop_icon "${CMAKE_MATCH_1}")
if(desktop_icon STREQUAL "")
    message(FATAL_ERROR "desktop entry has no absolute Icon path: ${desktop_file}")
endif()
if(NOT EXISTS "${STAGE}${desktop_icon}")
    message(FATAL_ERROR "desktop entry icon missing from install: ${desktop_icon}")
endif()

message(STATUS "install layout verified: ${STAGE}")
