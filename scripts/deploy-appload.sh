#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEVICE="${REMARKABLE_HOST:-root@10.11.99.1}"
PLUGIN="build/linuxfb-plugin/platforms/libqlinuxfb.so"

cd "${ROOT_DIR}"

for artifact in build/hn-reader "${PLUGIN}" device/hn-reader/external.manifest.json device/hn-reader/icon.png; do
    if [[ ! -f "${artifact}" ]]; then
        printf 'Required artifact missing: %s\n' "${artifact}" >&2
        printf 'Run scripts/build-docker.sh first.\n' >&2
        exit 1
    fi
done

if ! ssh "${DEVICE}" 'test -f /home/root/xovi/extensions.d/appload.so && test -f /home/root/shims/qtfb-shim.so'; then
    printf 'A working Xovi/AppLoad installation was not found on %s.\n' "${DEVICE}" >&2
    exit 1
fi

ssh "${DEVICE}" 'mkdir -p /home/root/hn-reader/plugins/platforms /home/root/xovi/exthome/appload/hn-reader'
scp build/hn-reader "${DEVICE}:/tmp/hn-reader.new"
scp "${PLUGIN}" "${DEVICE}:/tmp/libqlinuxfb.so.new"
scp device/hn-reader/external.manifest.json "${DEVICE}:/home/root/xovi/exthome/appload/hn-reader/external.manifest.json"
scp device/hn-reader/icon.png "${DEVICE}:/home/root/xovi/exthome/appload/hn-reader/icon.png"
ssh "${DEVICE}" 'chmod 755 /tmp/hn-reader.new /tmp/libqlinuxfb.so.new && mv /tmp/hn-reader.new /home/root/hn-reader/hn-reader && mv /tmp/libqlinuxfb.so.new /home/root/hn-reader/plugins/platforms/libqlinuxfb.so && systemctl restart xochitl.service'

printf 'HN Reader is installed in AppLoad on %s.\n' "${DEVICE}"
