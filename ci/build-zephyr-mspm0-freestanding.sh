#!/usr/bin/env bash
set -euo pipefail
set -x

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="${REPO_ROOT}/firmware/mspm0-gpo-extender/app"
BOARD_ROOT="${REPO_ROOT}/firmware/mspm0-gpo-extender"
BUILD_DIR="${REPO_ROOT}/build/mspm0-zephyr"

: "${ZEPHYR_BASE:?ZEPHYR_BASE must point to a Zephyr repository}"

if [ -z "${ZEPHYR_SDK_INSTALL_DIR:-}" ] && [ -d /opt/zephyr-toolchain/zephyr-sdk-0.17.4 ]; then
	export ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-toolchain/zephyr-sdk-0.17.4
fi
export ZEPHYR_TOOLCHAIN_VARIANT="${ZEPHYR_TOOLCHAIN_VARIANT:-zephyr}"
export ZEPHYR_SDK_INSTALL_DIR="${ZEPHYR_SDK_INSTALL_DIR:-/opt/zephyr-sdk}"
export CMAKE_PREFIX_PATH="${ZEPHYR_SDK_INSTALL_DIR}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"

rm -rf "${BUILD_DIR}"

west build -p always \
  -b jkembedded_mikrobus_hat_mspm0 \
  "${APP_DIR}" \
  --build-dir "${BUILD_DIR}" \
  -- \
  -DBOARD_ROOT="${BOARD_ROOT}"
