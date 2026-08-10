#!/bin/sh
set -eu

cleanup() {
    trap - EXIT INT TERM
    if [ -n "${app_pid:-}" ] && kill -0 "${app_pid}" 2>/dev/null; then
        kill "${app_pid}" 2>/dev/null || true
        wait "${app_pid}" 2>/dev/null || true
    fi
    systemctl start xochitl
}

trap cleanup EXIT INT TERM
systemctl stop xochitl
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"
export QT_QUICK_BACKEND=epaper
/home/root/hn-reader/hn-reader -platform epaper &
app_pid=$!
wait "${app_pid}"
