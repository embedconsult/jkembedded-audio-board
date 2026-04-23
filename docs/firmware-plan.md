# Firmware and Host Integration Plan

This document outlines the planned firmware, host utilities, and integration steps for the JK-Embedded Audio Board and mikroBUS HAT.

## MSPM0L1105TRGER GPO-extender
- **Goal**: Provide a simple I2C-accessible GPIO expander interface, or a fixed mux-profile image, to select mux configurations for each supported host platform.
- **Preferred stack**: Zephyr. This is no longer blocked: local board support, flashing, and GPIO control are working.
- **Bootloading**: The current working path uses `bb-imager-cli flash zepto` with line names `GPIO24` and `GPIO25` and the host bus `/dev/hat/mcu_i2c0`.
- **Host awareness**: Support optional host detection (GPIO line names or static profiles) to pre-select safe mux defaults.
- **Validated so far**:
  - `PA19` / `PA20` GPIO control at `J6`
  - `AN_SEL` / `INT_SEL` mux control via `AN -> INT` loopback
  - `PWM_SEL` mux control via `AN -> PWM` loopback
  - `CIPO_SEL_0` / `CIPO_SEL_1` mux control via `AN -> MISO` loopback
  - `RST_SEL` polarity using HAT pin `7` (`GPIO4`), which is tied to the `J7` signal through `R23` (`0 ohm`)
  - PCA9538-compatible I2C target emulation at `0x20`
  - Linux `gpio-pca953x` binding against the emulated target

## Host programmer utility
- **Language**: Rust or Crystal only; produce statically linked aarch64 binaries with no dynamic library dependencies.
- **Functionality**:
  - Detect the appropriate I2C controller via `/dev` symlinks when present; otherwise select per-board defaults.
  - Assert BOOTLOADER_SEL/RESET to enter the BSL and program the MSPM0 with the mux-control firmware image.
  - Offer command-line arguments to override board detection, I2C bus number, and GPIO line names.
- **Current working command**:

```sh
BB_IMAGER_CLI="${BB_IMAGER_CLI:-../bb-imager-rs/target/debug/bb-imager-cli}"
"${BB_IMAGER_CLI}" --verbose flash zepto \
  /path/to/zephyr.hex \
  --reset-gpio GPIO24 \
  --bsl-gpio GPIO25 \
  /dev/hat/mcu_i2c0
```
- **Targets**: BeagleY-AI, TI AM62 SK-EVM, TI AM68 SK-EVM, TI AM69 SK-EVM. BeaglePlay is not programmed because it does not use the mikroBUS HAT.

## Linux/Zephyr host integration
- **Clock generator (Si5351)**: Provide device-tree overlays and minimal configuration helpers to initialize VCXO control via DAC53002.
- **Audio codec**: Rely on mainline support for the TI AIC/TAS codec; supply DTS fragments to bind to the correct buses and clocks.
- **Configuration flow**: host binds PCA9538-compatible MSPM0 target -> mux selection via named GPIO lines -> clock generator setup -> codec registration.
- **Upstreaming**: Keep DTS overlays minimal and align node names/compatible strings with mainline conventions to ease upstream submission.

## Directory pointers
- `firmware/mspm0-gpo-extender/`: Firmware sources, build scripts, board configuration overlays, and usage documentation.
- `firmware/host-programmers/`: Programmer utility, board profiles, release binaries, and usage documentation.
- `firmware/host-integration/`: DTS overlays, helper scripts, and documentation for host configuration.
- `docs/`: Additional design notes, bring-up procedures, and any shared usage documentation.

## Next integration steps
1. Update host-side integration and default-profile code to use the measured `RST_SEL` polarity: `0 -> HAT pin 7 / GPIO4 / J7`, `1 -> HAT pin 35 / GPIO19`.
2. Apply the BeagleY-AI host overlay in normal boot flows so Linux exposes the emulated PCA9538 lines as `RST_SEL`, `PWM_SEL`, `CIPO_SEL_0`, `CIPO_SEL_1`, `AN_SEL`, and `INT_SEL` without manual `new_device` binding.
3. Decide the production MSPM0 firmware policy on top of the validated PCA9538 register model: pure host-controlled muxing, fixed power-on defaults, or host-profile-based defaults.
