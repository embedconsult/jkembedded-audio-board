# MSPM0 GPO-Extender Firmware

The MSPM0L1105TRGER manages mux control for the audio board and mikroBUS HAT.

## Objectives
- Expose mux selection over I2C with a GPO-expander-like register map.
- Provide optional host-detection logic to choose defaults based on SBC profiles.
- Keep the firmware small and configuration-driven; Zephyr preferred, TI SDK acceptable.

## Current state

- The shared Zephyr board `mikrobus_hat` is building and flashing successfully.
- Basic GPIO validation is working:
  - `firmware/blinky/` is the clean minimal board support / GPIO smoke test.
  - The `app/` image has been used to verify `PA19` / `PA20` at `J6`.
  - The `app/` image has also been used to verify `PA9` / `PA10` through the muxes with a host-side loopback test.
- PCA9538-style I2C target validation is working:
  - The current `app/` image emulates a `pca9538`-compatible target at `0x20`.
  - Direct `i2ctransfer` register reads/writes succeed.
  - A standard Linux `gpio-pca953x` client can bind at `0x20`.
  - With the `AN -> INT` loopback in place, Linux-driven expander bits 2 and 3 select the expected mux route.
- The BeagleY-AI flash path is now:

```sh
/home/beagle/bb-imager-rs/target/debug/bb-imager-cli --verbose flash zepto \
  /home/beagle/jkembedded-audio-board/build/mspm0-zephyr/zephyr/zephyr.hex \
  --reset-gpio GPIO24 \
  --bsl-gpio GPIO25 \
  /dev/hat/mcu_i2c0
```

- The current `app/` is still a validation app, not the final production mux-control firmware.

## Expected inputs/outputs
- **Inputs**: I2C commands from the host; BOOTLOADER_SEL and RESET to enter MSPM0 BSL.
- **Outputs**: Deterministic mux control lines for each supported host profile.

## Remaining work
- Validate selector polarity for `PA3`, `PA4`, `PA11`, and `PA15`
- Decide how host-profile policy should sit on top of the PCA9538 register model
- Add any host integration needed for line naming or board-specific defaults when binding the Linux `pca953x` client

## Zephyr board configuration

- **Board identifier**: `mikrobus_hat` (see `firmware/boards/arm/mikrobus_hat/`).
- **I2C role**: target/peripheral on `PA0`/`PA1`, currently emulating an 8-bit `pca9538` GPIO expander at `0x20` so existing Linux `pca953x` drivers can toggle the mux selects without changes.
- **I2C pins**: `PA0` = SDA (`HAT_SDA`), `PA1` = SCL (`HAT_SCL`).
- **Bootloader pins**: `PA18` = `MCU_BOOTLOADER_SEL`, `NRST` exposed via the reset net shared with the programming header.
- **Mux GPIO assignments (PCA9538 bit order)**:
  0. `PA3` → `RST_WRD_SEL`
  1. `PA4` → `PWM_BIT_SEL`
  2. `PA9` → `AN_DI_SEL`
  3. `PA10` → `INT_DO_SEL`
  4. `PA11` → `CIPO_CNT_SEL0`
  5. `PA15` → `CIPO_CNT_SEL1`
  6. reserved for future use
  7. reserved for future use

The shared board DTS in `firmware/boards/arm/mikrobus_hat/mikrobus_hat.dts` documents the current pinctrl and validation GPIO map. The current `app/` implementation uses the PCA9538 register model directly rather than the older EEPROM target shim.

## Build and CI smoke test

The Zephyr build smoke test lives in `app/` and targets `mikrobus_hat`. The repo `west.yml` now points Zephyr at the `audio-board` branch and imports only the required modules (`cmsis_6` and `hal_ti`). Run the same script locally from the repo root:

```sh
./ci/build-zephyr-mspm0.sh
```

The script initializes an isolated west workspace under `build/`, uses the Zephyr SDK toolchain if it is installed at `/opt/zephyr-sdk` (as in CI containers), and rebuilds from a clean tree to catch board or configuration regressions. Local bring-up on the BeagleY-AI host also works against `/home/beagle/zephyr` on the same branch. You can perform the same steps manually:

```sh
west init -l .
west update --narrow --fetch-opt=--depth=1
west build -p always \
  -b mikrobus_hat \
  "$PWD/firmware/mspm0-gpo-extender/app" \
  --build-dir "$PWD/build/mspm0-zephyr" \
  -- \
  -DBOARD_ROOT="$PWD/firmware"
```

CI also runs `clang-format` (using `.clang-format` from Zephyr) and `dtslint.py` on the board DTS before building.
