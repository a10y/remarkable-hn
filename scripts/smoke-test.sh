#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEVICE="${REMARKABLE_HOST:-root@10.11.99.1}"

cd "${ROOT_DIR}"
ssh "${DEVICE}" '/home/root/hn-reader/hn-reader --smoke-test'
