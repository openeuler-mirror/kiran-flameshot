# CTest 入口。
#
# 集成检查写成普通 CMake 脚本，因此也可以手动执行：
# cmake -DKFS_EXECUTABLE=... -P <script>。

add_test(NAME cmake.cli-smoke
    COMMAND ${CMAKE_COMMAND}
        -DKFS_EXECUTABLE=$<TARGET_FILE:kiran-flameshot>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/integration/cmake/cli-smoke.cmake)

# D-Bus 契约静态对照：接口 XML 与 FlameshotDBusAdapter 的槽签名必须一致。
# 生成代理只覆盖「CLI 调用 ↔ XML」，这条覆盖「XML ↔ 真实实现」。
add_test(NAME cmake.dbus-contract
    COMMAND ${CMAKE_COMMAND}
        -DKFS_XML=${PROJECT_SOURCE_DIR}/dbus/org.dharkael.Flameshot.xml
        -DKFS_ADAPTER_HEADER=${PROJECT_SOURCE_DIR}/src/core/flameshotdbusadapter.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/integration/cmake/dbus-contract.cmake)

# staging 安装到构建树内部，测试因此自包含，也不会触碰主机上的安装。
set(KFS_INSTALL_STAGE "${CMAKE_BINARY_DIR}/stage/install-layout")
add_test(NAME cmake.install-layout
    COMMAND ${CMAKE_COMMAND}
        -DKFS_BUILD_DIR=${CMAKE_BINARY_DIR}
        -DSTAGE=${KFS_INSTALL_STAGE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/integration/cmake/install-layout.cmake)

# captureutils 的行为对照。纯函数部分直接断言，光标与绘制部分
# 在 offscreen QPA 下用真实对象驱动，不需要替身，也不需要真实 X server。
add_executable(tst_captureutils
    integration/editor/tst_captureutils.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/captureutils.cpp
    # 光标形状来自 qrc；没有资源时 QCursor 构造会失败并回落为默认箭头，
    # 那样就测不到真实行为。
    ${PROJECT_SOURCE_DIR}/graphics.qrc)
target_include_directories(tst_captureutils PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_captureutils PRIVATE Qt5::Test Qt5::Widgets)
add_test(NAME integration.editor.captureutils COMMAND tst_captureutils)
set_tests_properties(integration.editor.captureutils PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# TextSizeButton 组件测试（offscreen，真实控件）。
add_executable(tst_textsizebutton
    integration/editor/tst_textsizebutton.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/text/textsizebutton.cpp)
target_include_directories(tst_textsizebutton PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_textsizebutton PRIVATE Qt5::Test Qt5::Widgets)
add_test(NAME integration.editor.textsizebutton COMMAND tst_textsizebutton)
set_tests_properties(integration.editor.textsizebutton PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# SaveMenuController 组件测试（offscreen，真实 QMenu 与 QAction）。
add_executable(tst_savemenucontroller
    integration/editor/tst_savemenucontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/widgets/capture/savemenucontroller.cpp)
target_include_directories(tst_savemenucontroller PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_savemenucontroller PRIVATE Qt5::Test Qt5::Widgets)
add_test(NAME integration.editor.savemenucontroller COMMAND tst_savemenucontroller)
set_tests_properties(integration.editor.savemenucontroller PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# SelectionHintLabel 组件测试（offscreen，真实父 widget 与几何）。
add_executable(tst_selectionhintlabel
    integration/editor/tst_selectionhintlabel.cpp
    ${PROJECT_SOURCE_DIR}/src/widgets/capture/selectionhintlabel.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/captureutils.cpp
    ${PROJECT_SOURCE_DIR}/graphics.qrc)
target_include_directories(tst_selectionhintlabel PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_selectionhintlabel PRIVATE Qt5::Test Qt5::Widgets)
add_test(NAME integration.editor.selectionhintlabel COMMAND tst_selectionhintlabel)
set_tests_properties(integration.editor.selectionhintlabel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# ZoomMagnifier。纯几何部分直接断言；绘制部分把真实控件渲染到
# QImage，而控件在构造期取整屏快照，因此与编辑器基线一样需要 Xvfb。
add_executable(tst_zoommagnifier
    integration/editor/tst_zoommagnifier.cpp
    ${PROJECT_SOURCE_DIR}/src/widgets/capture/zoommagnifier.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/captureutils.cpp
    ${PROJECT_SOURCE_DIR}/graphics.qrc)
target_include_directories(tst_zoommagnifier PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_zoommagnifier PRIVATE Qt5::Test Qt5::Widgets)
add_test(NAME integration.editor.zoommagnifier
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/integration/editor/run-legacy-editor.sh
            $<TARGET_FILE:tst_zoommagnifier>)
set_tests_properties(integration.editor.zoommagnifier PROPERTIES SKIP_RETURN_CODE 77)
