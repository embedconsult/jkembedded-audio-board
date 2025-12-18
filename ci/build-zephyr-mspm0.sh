#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
export PATH="$HOME/.local/bin:${PATH}"

BUILD_DIR="${REPO_ROOT}/build/mspm0-zephyr"
APP_DIR="${REPO_ROOT}/firmware/mspm0-gpo-extender/app"
BOARD_ROOT="${REPO_ROOT}/firmware/mspm0-gpo-extender"
ZEPHYR_BASE="${ZEPHYR_BASE:-${REPO_ROOT}/zephyr}"

export ZEPHYR_TOOLCHAIN_VARIANT="${ZEPHYR_TOOLCHAIN_VARIANT:-zephyr}"
export ZEPHYR_SDK_INSTALL_DIR="${ZEPHYR_SDK_INSTALL_DIR:-/opt/zephyr-sdk}"

rm -rf "${BUILD_DIR}"

west init -l "${REPO_ROOT}"

west update --narrow --fetch-opt=--depth=1
west zephyr-export

python3 -m pip install --user --upgrade pip
python3 -m pip install --user -r zephyr/scripts/requirements-base.txt

find "${BOARD_ROOT}" -type f \( -name '*.c' -o -name '*.h' \) -print0 \
	| xargs -0 -r clang-format --style=file --dry-run --Werror

python3 "${ZEPHYR_BASE}/scripts/dts/dtslint.py" \
	"${BOARD_ROOT}/boards/arm/jkembedded_mspm0/jkembedded_mspm0.dts"

west build -p always \
  -b jkembedded_mikrobus_hat_mspm0 \
  "${APP_DIR}" \
  --build-dir "${BUILD_DIR}" \
  -- \
  -DBOARD_ROOT="${BOARD_ROOT}"
