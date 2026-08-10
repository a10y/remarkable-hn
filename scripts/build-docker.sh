#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_VERSION="${REMARKABLE_SDK_VERSION:-5.6.75}"
SDK_IMAGE="${SDK_IMAGE:-hn-reader-sdk-host:22.04}"
SDK_VOLUME="${SDK_VOLUME:-hn-reader-sdk-${SDK_VERSION}}"

cd "${ROOT_DIR}"

if ! docker image inspect "${SDK_IMAGE}" >/dev/null 2>&1; then
    printf 'SDK host image missing. Run scripts/setup-sdk-container.sh first.\n' >&2
    exit 1
fi

if ! docker volume inspect "${SDK_VOLUME}" >/dev/null 2>&1; then
    printf 'SDK volume missing. Run scripts/setup-sdk-container.sh first.\n' >&2
    exit 1
fi

if ! docker run --rm --platform linux/amd64 -v "${SDK_VOLUME}:/sdk:ro" "${SDK_IMAGE}" \
    test -f /sdk/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi; then
    printf 'SDK volume missing or uninitialized. Run scripts/setup-sdk-container.sh first.\n' >&2
    exit 1
fi

docker run --rm --platform linux/amd64 \
    -v "${SDK_VOLUME}:/sdk:ro" \
    -v "${ROOT_DIR}:/workspace" \
    -w /workspace \
    "${SDK_IMAGE}" \
    bash -lc 'SDK_ROOT=/sdk scripts/build.sh && SDK_ROOT=/sdk scripts/build-linuxfb-plugin.sh'
