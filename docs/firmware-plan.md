# Firmware and Host Integration Plan

This document outlines the planned firmware, host utilities, and integration steps for the JK-Embedded Audio Board and mikroBUS HAT.

## MSPM0L1105TRGER GPO-extender
- **Goal**: Provide a simple I2C-accessible GPIO expander interface, or a fixed mux-profile image, to select mux configurations for each supported host platform.
- **Preferred stack**: Zephyr. This is no longer blocked: local board support, flashing, and GPIO control are working.
- **Bootloading**: The current working path uses `bb-imager-cli flash zepto` with line names `GPIO24` and `GPIO25` and the host bus `/dev/hat/mcu_i2c0`.
- **Host awareness**: Support optional host detection (GPIO line names or static profiles) to pre-select safe mux defaults.
- **Validated so far**:
  - `PA19` / `PA20` GPIO control at `J6`
  - `PA9` / `PA10` mux control via `AN -> INT` loopback
- **Still to validate**:
  - `PA3`, `PA4`, `PA11`, `PA15` selector polarity

## Host programmer utility
- **Language**: Rust or Crystal only; produce statically linked aarch64 binaries with no dynamic library dependencies.
- **Functionality**:
  - Detect the appropriate I2C controller via `/dev` symlinks when present; otherwise select per-board defaults.
  - Assert BOOTLOADER_SEL/RESET to enter the BSL and program the MSPM0 with the mux-control firmware image.
  - Offer command-line arguments to override board detection, I2C bus number, and GPIO line names.
- **Current working command**:

```sh
/home/beagle/bb-imager-rs/target/debug/bb-imager-cli --verbose flash zepto \
  /path/to/zephyr.hex \
  --reset-gpio GPIO24 \
  --bsl-gpio GPIO25 \
  /dev/hat/mcu_i2c0
```
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
1. Finish the remaining selector-polarity measurements (`PA3`, `PA4`, `PA11`, `PA15`) using the documented `gpioset` and probe procedure.
2. Apply and test the host-side `pca9538` integration so Linux exposes the MSPM0 lines as `RST_SEL`, `PWM_SEL`, `CIPO_SEL_0`, `CIPO_SEL_1`, `AN_SEL`, and `INT_SEL`.
3. Keep all MSPM0 Zephyr apps building from the shared `firmware/boards/arm/mikrobus_hat/` board root and extend CI coverage as new apps are added.
