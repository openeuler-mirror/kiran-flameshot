# bundled 第三方源码，不做改动。
#
# 这些库保留各自上游的许可证：
#   singleapplication   MIT（见 singleapplication/LICENSE）
#   Qt-Color-Widgets    LGPL-3.0-or-later with exception（见 Qt-Color-Widgets/COPYING）

add_library(kfs-singleapplication STATIC
    singleapplication/singleapplication.cpp
    singleapplication/singleapplication.h
    singleapplication/singleapplication_p.h)
target_include_directories(kfs-singleapplication PUBLIC singleapplication)
target_link_libraries(kfs-singleapplication PUBLIC
    Qt5::Core
    Qt5::Network
    Qt5::Widgets)

# singleapplication.h 在没有该宏时默认继承 QCoreApplication，而本项目需要
# QApplication：宏必须对库自身和所有使用者取同一值，否则同一个类在不同编译单元
# 里基类不同（ODR 违反），表现为无参启动时创建 QMenu 直接 abort。
target_compile_definitions(kfs-singleapplication PUBLIC
    QAPPLICATION_CLASS=QApplication)

add_library(kfs-color-widgets STATIC
    Qt-Color-Widgets/src/color_utils.cpp
    Qt-Color-Widgets/src/color_wheel.cpp
    Qt-Color-Widgets/include/color_wheel.hpp
    Qt-Color-Widgets/src/color_widgets.qrc)
target_include_directories(kfs-color-widgets PUBLIC
    Qt-Color-Widgets/include Qt-Color-Widgets/src)
target_link_libraries(kfs-color-widgets PUBLIC Qt5::Gui Qt5::Widgets)
