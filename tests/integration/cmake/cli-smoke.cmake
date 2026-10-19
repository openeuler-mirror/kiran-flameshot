# CLI 冒烟与行为基线检查。
#
# 用法：
#   cmake -DKFS_EXECUTABLE=<可执行文件路径> -P cli-smoke.cmake
#
# 拆分为 cli/daemon 双入口后，这里同时承担"对外行为"的机器可读基线：
# stdout 与退出码是契约（退出码恒 0，含解析失败与 checker 失败）；stderr 自本轮起
# 承载 klog 诊断日志，因此这里只断言 stdout 内容与退出码，不断言 stderr。
# Usage 行包含 argv[0] 全文，因此只断言其中固定的片段，不断言程序路径。

if(NOT DEFINED KFS_EXECUTABLE)
    message(FATAL_ERROR "KFS_EXECUTABLE is required")
endif()
if(NOT EXISTS "${KFS_EXECUTABLE}")
    message(FATAL_ERROR "missing executable: ${KFS_EXECUTABLE}")
endif()

# 运行一次 CLI，结果放入 KFS_RC / KFS_OUT / KFS_ERR。
function(kfs_run)
    execute_process(COMMAND "${KFS_EXECUTABLE}" ${ARGN}
                    RESULT_VARIABLE _rc
                    OUTPUT_VARIABLE _out
                    ERROR_VARIABLE _err)
    set(KFS_RC "${_rc}" PARENT_SCOPE)
    set(KFS_OUT "${_out}" PARENT_SCOPE)
    set(KFS_ERR "${_err}" PARENT_SCOPE)
endfunction()

function(kfs_expect_rc_zero _case)
    if(NOT KFS_RC EQUAL 0)
        message(FATAL_ERROR "${_case}: 退出码必须为 0，实际为 ${KFS_RC}")
    endif()
endfunction()

function(kfs_expect_stdout _case _pattern)
    if(NOT KFS_OUT MATCHES "${_pattern}")
        message(FATAL_ERROR "${_case}: stdout 缺少 ${_pattern}；实际输出: ${KFS_OUT}")
    endif()
endfunction()

# 1) --version：版本号与 Qt 版本，全部走 stdout。
kfs_run(--version)
kfs_expect_rc_zero("--version")
kfs_expect_stdout("--version" "Flameshot [0-9]+\\.[0-9]+\\.[0-9]+")
kfs_expect_stdout("--version" "Compiled with Qt")

# 2) --help：根参数域的用法与全部子参数名，顺序即声明顺序。
kfs_run(--help)
kfs_expect_rc_zero("--help")
kfs_expect_stdout("--help" "\\[kiran-flameshot-options\\] \\[arguments\\]")
kfs_expect_stdout("--help" "-h, --help")
kfs_expect_stdout("--help" "-v, --version")
foreach(_argument gui screen full window launcher config)
    kfs_expect_stdout("--help" "  ${_argument} +[A-Z]")
endforeach()

# 3) full --help：子参数域用法与 full 的全部选项。
kfs_run(full --help)
kfs_expect_rc_zero("full --help")
kfs_expect_stdout("full --help" "full \\[full-options\\]")
foreach(_option "-p, --path" "-c, --clipboard" "-d, --delay" "-r, --raw")
    kfs_expect_stdout("full --help" "${_option}")
endforeach()

# 4) 未知参数：解析失败，但退出码仍为 0，错误走 stdout。
kfs_run(--bogus)
kfs_expect_rc_zero("--bogus")
kfs_expect_stdout("--bogus" "The option 'bogus' is not a valid option")

# 5) checker 失败：路径不存在，退出码仍为 0，错误走 stdout。
kfs_run(full -p /nonexistent-kfs-cli-contract)
kfs_expect_rc_zero("full -p <missing>")
kfs_expect_stdout("full -p <missing>" "Invalid path, it must be a real path in the system\\.")
