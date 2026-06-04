#!/usr/bin/env bash
set -euo pipefail

# Ensure the script runs from the repository root even if invoked elsewhere
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}/.."

required_paths=(
  "AGENTS.md"
  "docs/firmware-plan.md"
  "firmware/README.md"
  "firmware/mspm0-gpo-extender/README.md"
  "firmware/host-programmers/README.md"
  "firmware/host-integration/README.md"
  "firmware/host-integration/linux/set-mux-profile.sh"
  "firmware/host-integration/linux/beagley-ai/mspm0-pca9538-gpio.dts"
  "firmware/host-integration/linux/sk-am62/mspm0-pca9538-gpio.dts"
  "firmware/host-integration/linux/sk-am68/mspm0-pca9538-gpio.dts"
  "firmware/host-integration/linux/sk-am69/mspm0-pca9538-gpio.dts"
)

missing=()
for path in "${required_paths[@]}"; do
  if [ ! -e "$path" ]; then
    missing+=("$path")
  fi
done

if [ ${#missing[@]} -ne 0 ]; then
  echo "Missing required project files:" >&2
  printf ' - %s\n' "${missing[@]}" >&2
  exit 1
fi

echo "Project structure check passed."
