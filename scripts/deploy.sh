#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEVICE="${REMARKABLE_HOST:-root@10.11.99.1}"

cd "${ROOT_DIR}"

if [[ ! -x build/hn-reader ]]; then
    printf 'Build artifact missing. Run scripts/build.sh first.\n' >&2
    exit 1
fi

scp build/hn-reader "${DEVICE}:/tmp/hn-reader.new"
scp scripts/run-on-device.sh "${DEVICE}:/tmp/hn-reader-run.new"
ssh "${DEVICE}" 'mkdir -p /home/root/hn-reader && chmod 755 /tmp/hn-reader.new /tmp/hn-reader-run.new && mv /tmp/hn-reader.new /home/root/hn-reader/hn-reader && mv /tmp/hn-reader-run.new /home/root/hn-reader/run.sh'

printf 'Installed. Launch with: ssh -t %s /home/root/hn-reader/run.sh\n' "${DEVICE}"
