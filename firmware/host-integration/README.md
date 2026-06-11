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

Do not start here for fresh-system bring-up. The initial reproduction point is
the raw build/flash/raw-I2C flow in `bringup.md`, before any overlay or manual
Linux `pca953x` binding is introduced.

### Host-board overlays

Use the matching `jkembedded-mikrobus-hat.dts` overlay under
`firmware/host-integration/linux/<host>/` to instantiate the Linux
`gpio-pca953x` driver with stable line names and base HAT GPIO line names.

Available overlays:

| Host board | Overlay | I2C target |
| ---------- | ------- | ---------- |
| BeagleY-AI / AM67A | `linux/beagley-ai/jkembedded-mikrobus-hat.dts` | observed live path `/bus@f0000/bus@4000000/i2c@4900000` |
| TI AM62 SK-EVM | `linux/sk-am62/jkembedded-mikrobus-hat.dts` | `&main_i2c2` |
| TI AM68 SK-EVM | `linux/sk-am68/jkembedded-mikrobus-hat.dts` | `&main_i2c4` |
| TI AM69 SK-EVM | `linux/sk-am69/jkembedded-mikrobus-hat.dts` | `&mcu_i2c0` |

Each overlay exposes these selector lines:

- `RST_SEL`
- `PWM_SEL`
- `AN_SEL`
- `INT_SEL`
- `CIPO_SEL_0`
- `CIPO_SEL_1`

These base HAT overlays name the MSPM0 selector lines and host-side HAT GPIO
lines, but they do not drive selector GPIOs to any target profile. Apply a
separate mikroBUS add-on-board overlay, or use the helper below, when a board
needs specific mux settings. The BeagleY-AI overlay keeps the existing live
`i2c-1` `target-path` because that path was validated on hardware. The SK-EVM
overlays use upstream board labels so they follow the base DTS I2C bus layout
used by each target kernel.

### Host mux profile helper

Use `firmware/host-integration/linux/set-mux-profile.sh` after the MSPM0
`pca9538` target is reachable to drive all six selector lines for a supported
host-board profile in one command. The helper uses the overlay-provided line
names when available and otherwise falls back to the detected `pca9538`
`gpiochip` offsets. It binds a temporary Linux `pca9538` client at `0x20`
on `i2c-1` by default when no client exists yet. The base HAT overlays do not
hog the selector GPIOs, so the helper can be used after the overlay loads when
you need to change
profiles interactively.

```console
./firmware/host-integration/linux/set-mux-profile.sh --host byai-am67a
./firmware/host-integration/linux/set-mux-profile.sh --host sk-am62
./firmware/host-integration/linux/set-mux-profile.sh --host sk-am68
./firmware/host-integration/linux/set-mux-profile.sh --host sk-am69
```

Pass `--i2c-bus <bus>` or set `I2C_BUS=<bus>` when the MSPM0 target is on a
different Linux I2C adapter. Use `--dry-run` to print the selected values
without touching GPIOs.

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

- `AN -> RST`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-rst
```

Probe or read HAT pin `35` (`GPIO19`) and HAT pin `7` (`GPIO4`). HAT pin `7`
is tied to the same signal that is also brought out at `J7` through `R23`
(`0 ohm`), so it is the preferred validation point on this PCB revision.

Expected output:

```console
AN->RST low state (expect GPIO19=1 GPIO4=0): 1 0
AN->RST high state (expect GPIO19=0 GPIO4=1): 0 1
```
