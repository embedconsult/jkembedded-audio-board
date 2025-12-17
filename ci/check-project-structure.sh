#!/usr/bin/env bash
set -euo pipefail

required_paths=(
  "AGENTS.md"
  "docs/firmware-plan.md"
  "firmware/README.md"
  "firmware/mspm0-gpo-extender/README.md"
  "firmware/host-programmers/README.md"
  "firmware/host-integration/README.md"
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
