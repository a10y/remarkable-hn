#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_VERSION="${REMARKABLE_SDK_VERSION:-5.6.75}"
SDK_INSTALLER="${1:-${REMARKABLE_SDK_INSTALLER:-}}"
SDK_IMAGE="${SDK_IMAGE:-hn-reader-sdk-host:22.04}"
SDK_VOLUME="${SDK_VOLUME:-hn-reader-sdk-${SDK_VERSION}}"
ENVIRONMENT="/sdk/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi"

if [[ -z "${SDK_INSTALLER}" || ! -f "${SDK_INSTALLER}" ]]; then
    printf 'Usage: %s /path/to/remarkable-production-image-*-rm2-public-x86_64-toolchain.sh\n' "$0" >&2
    exit 1
fi

cd "${ROOT_DIR}"
docker build --platform linux/amd64 -f Dockerfile.sdk -t "${SDK_IMAGE}" .
docker volume create "${SDK_VOLUME}" >/dev/null

if docker run --rm --platform linux/amd64 -v "${SDK_VOLUME}:/sdk:ro" "${SDK_IMAGE}" test -f "${ENVIRONMENT}"; then
    printf 'SDK volume %s is already initialized.\n' "${SDK_VOLUME}"
    exit 0
fi

docker run --rm --platform linux/amd64 \
    -v "$(realpath "${SDK_INSTALLER}"):/installer/sdk.sh:ro" \
    -v "${SDK_VOLUME}:/sdk" \
    "${SDK_IMAGE}" sh /installer/sdk.sh -y -d /sdk

printf 'Initialized SDK volume %s.\n' "${SDK_VOLUME}"
