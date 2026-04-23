# Host Integration Assets

Device-tree overlays, configuration helpers, and example code for integrating the audio board and mikroBUS HAT with supported hosts.

## Focus areas
- Si5351 clock generator setup with DAC53002 VCXO control.
- Audio codec wiring and driver bindings using mainline-compatible DTS fragments.
- Mux control defaults coordinated with the MSPM0 GPO-extender firmware.

## Deliverables
- DTS overlays per supported host (BeagleY-AI, TI AM62/68/69 SK-EVM).
- Minimal configuration utilities or scripts to apply overlays and initialize clocks.
- Notes on upstreaming overlays/driver tweaks to mainline Linux/Zephyr.

## Current MSPM0 host integration

The MSPM0 now emulates a `pca9538`-compatible GPIO expander at `0x20` on the
host-visible I2C bus `/dev/hat/mcu_i2c0` (`i2c-1` on the current BeagleY-AI
setup).

### BeagleY-AI overlay

Use `firmware/host-integration/linux/beagley-ai/mspm0-pca9538-gpio.dts`
to instantiate the Linux `gpio-pca953x` driver with stable line names:

- `RST_SEL`
- `PWM_SEL`
- `AN_SEL`
- `INT_SEL`
- `CIPO_SEL_0`
- `CIPO_SEL_1`

The overlay targets the live BeagleY-AI `i2c-1` controller path observed on
this system:

- `/bus@f0000/bus@4000000/i2c@4900000`

### Validation recipe

Current proven setup:

1. Remove the audio board.
2. Short mikroBUS `AN` to `INT`.
3. Bind the `pca9538` client either by overlay at boot or temporarily with:

```console
echo pca9538 0x20 | sudo tee /sys/bus/i2c/devices/i2c-1/new_device
```

4. Run the scripted validator:

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-int
```

Manual hold-and-probe mode is available too:

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh hold an-int low
./firmware/host-integration/linux/beagley-ai/validate-mux.sh hold an-int high
```

The script uses the named selector lines when the overlay is active and falls
back to `gpiochip3` offsets otherwise.

### Remaining selector setups

Use these jumper configurations to finish validating the remaining mux controls:

- `AN -> PWM`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-pwm
```

Expected output on the current validated board:

```console
AN->PWM low state (expect GPIO18=1 GPIO17=0): 1 0
AN->PWM high state (expect GPIO18=0 GPIO17=1): 0 1
```

- `AN -> MISO`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-miso
```

Expected output on the current validated board:

```console
AN->MISO state 0 (CIPO_SEL_0=0 CIPO_SEL_1=0) -> GPIO16 GPIO20 GPIO17: 1 1 1
AN->MISO state 1 (CIPO_SEL_0=0 CIPO_SEL_1=1) -> GPIO16 GPIO20 GPIO17: 1 0 1
AN->MISO state 2 (CIPO_SEL_0=1 CIPO_SEL_1=0) -> GPIO16 GPIO20 GPIO17: 0 1 1
AN->MISO state 3 (CIPO_SEL_0=1 CIPO_SEL_1=1) -> GPIO16 GPIO20 GPIO17: 1 1 0
```

- `AN -> RST` and `J7/7 -> PWM`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-rst-j7-pwm
```

The `RST_SEL` fixture is currently not trustworthy on this PCB revision because
`J7` is undersized. The measured output so far is:

```console
AN->RST / J7->PWM low state (measured GPIO19 GPIO18): 1 1
AN->RST / J7->PWM high state (measured GPIO19 GPIO18): 0 1
```
