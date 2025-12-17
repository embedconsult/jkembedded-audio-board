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
- Add build scripts and CI integration.
