# Firmware and Host Integration Plan

This document outlines the planned firmware, host utilities, and integration steps for the JK-Embedded Audio Board and mikroBUS HAT.

## MSPM0L1105TRGER GPO-extender
- **Goal**: Provide a simple I2C-accessible GPIO expander interface to select mux configurations for each supported host platform.
- **Preferred stack**: Zephyr (minimal footprint with device-tree-based pin mapping). Fall back to TI SDK if Zephyr bring-up blocks delivery.
- **Bootloading**: Enter the MSPM0 ROM BSL via BOOTLOADER_SEL and RESET lines for programming and updates only; normal operation should run the mux-control firmware without requiring bootloader entry.
- **Host awareness**: Support optional host detection (GPIO line names or static profiles) to pre-select safe mux defaults.

## Host programmer utility
- **Language**: Rust or Crystal only; produce statically linked aarch64 binaries with no dynamic library dependencies.
- **Functionality**:
  - Detect the appropriate I2C controller via `/dev` symlinks when present; otherwise select per-board defaults.
  - Assert BOOTLOADER_SEL/RESET to enter the BSL and program the MSPM0 with the mux-control firmware image.
  - Offer command-line arguments to override board detection, I2C bus number, and GPIO line names.
- **Targets**: BeagleY-AI, TI AM62 SK-EVM, TI AM68 SK-EVM, TI AM69 SK-EVM. BeaglePlay is not programmed because it does not use the mikroBUS HAT.

## Linux/Zephyr host integration
- **Clock generator (Si5351)**: Provide device-tree overlays and minimal configuration helpers to initialize VCXO control via DAC53002.
- **Audio codec**: Rely on mainline support for the TI AIC/TAS codec; supply DTS fragments to bind to the correct buses and clocks.
- **Configuration flow**: DTS overlay -> mux selection via MSPM0 -> clock generator setup -> codec registration.
- **Upstreaming**: Keep DTS overlays minimal and align node names/compatible strings with mainline conventions to ease upstream submission.

## Directory pointers
- `firmware/mspm0-gpo-extender/`: Firmware sources, build scripts, board configuration overlays, and usage documentation.
- `firmware/host-programmers/`: Programmer utility, board profiles, release binaries, and usage documentation.
- `firmware/host-integration/`: DTS overlays, helper scripts, and documentation for host configuration.
- `docs/`: Additional design notes, bring-up procedures, and any shared usage documentation.

## Next integration steps
1. Define Zephyr board overlays and pinmux for the MSPM0 design, including BOOTLOADER_SEL and RESET handling.
2. Draft the programmer utility CLI and board detection matrix; validate GPIO line naming on each target host.
3. Create initial DTS overlays for Si5351 and DAC53002 usage on the target hosts.
4. Add CI jobs to build/test the firmware and programmer utility once sources are added.
