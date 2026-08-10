#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEVICE="${REMARKABLE_HOST:-root@10.11.99.1}"

cd "${ROOT_DIR}"

if ! ssh "${DEVICE}" 'test -x /home/root/xovi/start && test -x /home/root/xovi/stock'; then
    printf 'A working Xovi installation was not found on %s.\n' "${DEVICE}" >&2
    exit 1
fi

scp device/xovi-persistent.service "${DEVICE}:/etc/systemd/system/xovi-persistent.service"
scp device/xovi-recovery.service "${DEVICE}:/etc/systemd/system/xovi-recovery.service"
scp device/recover-stock.sh "${DEVICE}:/home/root/xovi/recover-stock.sh"
ssh "${DEVICE}" 'chmod 755 /home/root/xovi/recover-stock.sh && systemctl daemon-reload && systemctl enable --now xovi-persistent.service'

printf 'Persistent Xovi startup is enabled on %s.\n' "${DEVICE}"
