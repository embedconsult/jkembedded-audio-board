# Firmware and Utilities

This directory contains all firmware and host-side utilities for the JK-Embedded Audio Board and mikroBUS HAT.

## Contents
- `mspm0-gpo-extender/`: Firmware for the MSPM0L1105TRGER mux controller. Zephyr is preferred; TI SDK is acceptable if Zephyr is blocked.
- `host-programmers/`: Rust/Crystal programmer utilities for aarch64 Linux hosts to load the mux-control firmware over I2C using the MSPM0 ROM BSL.
- `host-integration/`: Device-tree overlays, configuration helpers, and example host code (Linux/Zephyr) for the clock generator and audio codec.

## Guidelines
- Keep host utilities self-contained with static linking; avoid Python or PyPI dependencies.
- Update CI workflows when adding build targets or tests for these components.
- Document board-specific GPIO and I2C mappings alongside the corresponding code.
