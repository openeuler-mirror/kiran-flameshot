# kiran-flameshot 的版本取值。
#
# 优先级：
#   1. KIRAN_FLAMESHOT_VERSION_OVERRIDE  （发行版构建注入的发布版本串）
#   2. PROJECT_VERSION                   （来自 project(... VERSION ...)）
#
# 版本取值不得依赖 .git 目录，否则源码包（tarball）构建会失效。
if(DEFINED KIRAN_FLAMESHOT_VERSION_OVERRIDE AND NOT KIRAN_FLAMESHOT_VERSION_OVERRIDE STREQUAL "")
    set(KIRAN_FLAMESHOT_VERSION "${KIRAN_FLAMESHOT_VERSION_OVERRIDE}")
else()
    set(KIRAN_FLAMESHOT_VERSION "${PROJECT_VERSION}")
endif()

if(NOT KIRAN_FLAMESHOT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+")
    message(FATAL_ERROR
        "invalid KIRAN_FLAMESHOT_VERSION '${KIRAN_FLAMESHOT_VERSION}': "
        "expected a numeric major.minor.patch prefix")
endif()

message(STATUS "kiran-flameshot version: ${KIRAN_FLAMESHOT_VERSION}")
