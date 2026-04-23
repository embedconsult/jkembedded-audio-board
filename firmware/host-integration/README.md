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

Use [mspm0-pca9538-gpio.dts](/home/beagle/jkembedded-audio-board/firmware/host-integration/linux/beagley-ai/mspm0-pca9538-gpio.dts)
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

4. Until the overlay is applied, drive bits 2 and 3 through `gpiochip3`.
5. After the overlay is applied, use the line names `AN_SEL` and `INT_SEL`.

Example host-side route checks:

```console
gpioset -C host-a GPIO13=0 GPIO20=1
gpioset -C mux-low -c gpiochip3 2=0 3=0
# Expect GPIO16 low, GPIO21 high

gpioset -C host-b GPIO13=1 GPIO20=0
gpioset -C mux-high -c gpiochip3 2=1 3=1
# Expect GPIO16 high, GPIO21 low
```

When the overlay is active, replace the `gpiochip3` offsets with:

```console
gpioset AN_SEL=0 INT_SEL=0
gpioset AN_SEL=1 INT_SEL=1
```
