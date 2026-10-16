#!/usr/bin/env bash
# 旧编辑器可见行为基线的运行脚本。
#
# 为什么需要它：旧 CaptureWidget 在构造期用 ScreenGrabber 自己抓屏，offscreen 平台
# 下 QScreen::grabWindow(0) 返回空 pixmap，widget 会立即 close。因此测试必须跑在
# 真实 X server 上；这里用 Xvfb 起一个与用户桌面无关的虚拟 server。
#
# 用法：run-legacy-editor.sh <test-binary>
#
# 退出码：77 表示环境不具备（没有 xvfb-run），由 CTest 记为 Skipped，不伪造通过。
set -euo pipefail

binary="${1:?usage: run-legacy-editor.sh <test-binary>}"

if ! command -v xvfb-run >/dev/null 2>&1; then
    echo "SKIP 找不到 xvfb-run，无法提供虚拟 X server" >&2
    exit 77
fi

# 所有可写状态都落在临时目录里，绝不触碰用户的配置、桌面或自动启动项。
sandbox="$(mktemp -d)"
trap 'rm -rf "$sandbox"' EXIT

mkdir -p "$sandbox/home/.config" "$sandbox/home/.cache" \
         "$sandbox/home/.local/share" "$sandbox/run"
chmod 700 "$sandbox/run"

exec xvfb-run -a -s "-screen 0 1280x800x24" \
    env HOME="$sandbox/home" \
        XDG_CONFIG_HOME="$sandbox/home/.config" \
        XDG_CACHE_HOME="$sandbox/home/.cache" \
        XDG_DATA_HOME="$sandbox/home/.local/share" \
        XDG_RUNTIME_DIR="$sandbox/run" \
        QT_QPA_PLATFORM=xcb \
        "$binary" "${@:2}"
