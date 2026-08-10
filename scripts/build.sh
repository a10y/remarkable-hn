#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_VERSION="${REMARKABLE_SDK_VERSION:-5.6.75}"
SDK_ROOT="${SDK_ROOT:-/opt/codex/rm2/${SDK_VERSION}}"
ENVIRONMENT="${SDK_ROOT}/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi"

cd "${ROOT_DIR}"

if [[ ! -f "${ENVIRONMENT}" ]]; then
    printf 'SDK environment not found: %s\n' "${ENVIRONMENT}" >&2
    exit 1
fi

# shellcheck source=/dev/null
source "${ENVIRONMENT}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
