# MSPM0 GPO-Extender Firmware

The MSPM0L1105TRGER manages mux control for the audio board and mikroBUS HAT.

## Objectives
- Expose mux selection over I2C with a GPO-expander-like register map.
- Provide optional host-detection logic to choose defaults based on SBC profiles.
- Keep the firmware small and configuration-driven; Zephyr preferred, TI SDK acceptable.

## Expected inputs/outputs
- **Inputs**: I2C commands from the host; BOOTLOADER_SEL and RESET to enter MSPM0 BSL.
- **Outputs**: Deterministic mux control lines for each supported host profile.

## TODO
- Define Zephyr board configuration and pin assignments.
- Establish the GPO register map and host profile table.

## Zephyr board configuration

- **Board identifier**: `jkembedded_mikrobus_hat_mspm0` (see `boards/arm/jkembedded_mspm0/`).
- **I2C role**: target/peripheral that emulates an 8-bit `pca9538` GPIO expander at address `0x20` so existing Linux `pca953x` drivers can toggle the mux selects without changes.
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

The device tree in `boards/arm/jkembedded_mspm0/jkembedded_mspm0.dts` documents pinctrl defaults for I2C and the mux GPIO map. The PCA9538 compatibility is consumed on the host side; the firmware should expose that register map over I2C so existing `pca953x` drivers work without modification.

## Build and CI smoke test

The Zephyr build smoke test lives in `app/` and targets `jkembedded_mikrobus_hat_mspm0`. CI uses `ci/build-zephyr-mspm0.sh` to fetch Zephyr v4.0.0, point `BOARD_ROOT` at this repository, and build the stub firmware with the `zephyrprojectrtos/zephyr-build:v0.28.6` container. Run the same script locally from the repo root:

```sh
./ci/build-zephyr-mspm0.sh
```

The script initializes an isolated west workspace under `build/`, uses the Zephyr SDK toolchain if it is installed at `/opt/zephyr-sdk` (as in CI containers), and rebuilds from a clean tree to catch board or configuration regressions.
