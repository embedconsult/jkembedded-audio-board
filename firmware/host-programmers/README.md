# Host Programmer Utilities

Utilities in this directory program the MSPM0L1105TRGER mux-control firmware from supported hosts.

## Requirements
- Rust or Crystal implementation only; produce statically linked aarch64 Linux binaries.
- No Python or PyPI dependencies.
- Use GPIO line names and I2C controller symlinks when available; allow CLI overrides and static board profiles otherwise.

## Target hosts
- BeagleY-AI
- TI AM62 SK-EVM
- TI AM68 SK-EVM
- TI AM69 SK-EVM

BeaglePlay is excluded because it does not use the mikroBUS HAT.

## Planned features
- Board detection and safe default mux selection.
- BSL entry via BOOTLOADER_SEL and RESET control.
- Firmware image flashing and verification over I2C.
- Optional dry-run mode for validation during production testing.
