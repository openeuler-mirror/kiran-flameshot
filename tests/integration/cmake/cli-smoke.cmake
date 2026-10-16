# CLI 冒烟检查：构建出的可执行文件必须能响应 --version。
#
# 用法：
#   cmake -DKFS_EXECUTABLE=<可执行文件路径> -P cli-smoke.cmake
if(NOT DEFINED KFS_EXECUTABLE)
    message(FATAL_ERROR "KFS_EXECUTABLE is required")
endif()
if(NOT EXISTS "${KFS_EXECUTABLE}")
    message(FATAL_ERROR "missing executable: ${KFS_EXECUTABLE}")
endif()
execute_process(COMMAND "${KFS_EXECUTABLE}" --version
                RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT rc EQUAL 0 OR NOT out MATCHES "[Kk]iran-flameshot|Flameshot")
    message(FATAL_ERROR "--version failed: rc=${rc}; out=${out}; err=${err}")
endif()
