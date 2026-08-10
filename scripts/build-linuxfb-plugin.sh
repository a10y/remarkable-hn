#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_VERSION="${REMARKABLE_SDK_VERSION:-5.6.75}"
SDK_ROOT="${SDK_ROOT:-/opt/codex/rm2/${SDK_VERSION}}"
QTBASE_SOURCE_DIR="${QTBASE_SOURCE_DIR:-build/qtbase-v6.8.2}"
ENVIRONMENT="${SDK_ROOT}/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi"

cd "${ROOT_DIR}"

if [[ ! -f "${ENVIRONMENT}" ]]; then
    printf 'SDK environment not found: %s\n' "${ENVIRONMENT}" >&2
    exit 1
fi

mkdir -p build
if [[ ! -d "${QTBASE_SOURCE_DIR}/src/plugins/platforms/linuxfb" ]]; then
    git clone --depth 1 --branch v6.8.2 https://github.com/qt/qtbase.git "${QTBASE_SOURCE_DIR}"
fi

# shellcheck source=/dev/null
source "${ENVIRONMENT}"
cmake -S linuxfb-plugin -B build/linuxfb-plugin \
    -DCMAKE_BUILD_TYPE=Release \
    -DQTBASE_SOURCE_DIR="$(realpath "${QTBASE_SOURCE_DIR}")"
cmake --build build/linuxfb-plugin --parallel
