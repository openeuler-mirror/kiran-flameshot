# legacy 应用目标的源码清单。
#
# 用 glob 收集而不是逐文件列举：旧源码树有 80 多个实现文件，手工清单既啰嗦又容易
# 漏项——迁移到 CMake 时的第一版清单就漏掉了 src/utils/waylandutils.cpp。需要排除的
# 文件在下面显式声明，排除理由都具体且可验证。
#
# 本工程最低支持 CMake 3.6，没有 file(GLOB ... CONFIGURE_DEPENDS)（3.12 才引入），
# 因此 glob 结果在配置期固定：新增或删除实现文件后需要重新运行一次 cmake。

set(KFS_LEGACY_ROOT "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE KFS_APP_SOURCES "${KFS_LEGACY_ROOT}/*.cpp")

# src/third-party/ 由独立静态库目标编译（kfs-singleapplication、kfs-color-widgets），
# 不能进入 legacy 应用，否则符号重复定义。
file(GLOB_RECURSE _kfs_third_party_sources "${KFS_LEGACY_ROOT}/third-party/*.cpp")
list(REMOVE_ITEM KFS_APP_SOURCES ${_kfs_third_party_sources})

# 有意排除的实现文件：
#   utils/waylandutils.cpp  死文件：全仓无任何引用
list(REMOVE_ITEM KFS_APP_SOURCES
    "${KFS_LEGACY_ROOT}/utils/waylandutils.cpp")

# 没有同名 .cpp 的纯头文件 QObject 类：AUTOMOC 只为源码清单中的文件生成 moc，
# 因此这几项必须显式列出。
# src/tools/capturetool.h 是所有编辑器工具的基类。
set(KFS_APP_MOC_HEADERS
    ${KFS_LEGACY_ROOT}/tools/capturetool.h
    ${KFS_LEGACY_ROOT}/tools/linebutton.h
    ${KFS_LEGACY_ROOT}/tools/rectbutton.h
    ${KFS_LEGACY_ROOT}/tools/toolbutton.h)
