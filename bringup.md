# Bring-up notes/checklist

## Audio add-on

### Initial power

#### 3.3V only

- [x] 3.3V: 2-7mA
- [x] PWM/BIT: 0V
- [x] RST/WRD: 0V
- [x] PICO/TS: 0V
- [x] POCI/CNT: 0V
- [x] CS: 3.3V
- [x] AN/DI: 0V
- [x] INT/DO: 0V
- [x] SCL: 0V
- [x] SDA: 0V
- [x] 5V: 0V
- [x] TP1: 0V
- [x] Y1: 0V
- [x] MCLK: 0V

#### 5V only

- [x] 5V: 25-27mA
- [x] 3V3-OSC: 3.32V, some noise
- [x] PWM/BIT: 0V
- [x] RST/WRD: 0V
- [x] PICO/TS: 0V
- [x] POCI/CNT: 0V
- [x] CS: 3.3V
- [x] AN/DI: 0V
- [x] INT/DO: 0V
- [x] SCL: 0V
- [x] SDA: 0V
- [x] 5V: 0V
- [x] TP1: 0V (with noise)
- [x] Y1: 27MHz sine wave, with some anomolies
- [x] MCLK: 0V

#### I2C

Providing 3.3V and 5V

```console
beagle@beagle:~$ ls -l /dev/play/mikrobus/i2c 
lrwxrwxrwx 1 root root 11 Feb 14 17:57 /dev/play/mikrobus/i2c -> ../../i2c-3
beagle@beagle:~$ i2cdetect -y -r 3
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- 47 48 -- -- -- -- -- -- -- 
50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: 60 -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --             
```

### MSPM0 Zephyr bring-up and mux verification

This section captures the current working state, not the older failed
bring-up attempts.

#### Host setup used

- BeagleY-AI host
- Local Zephyr workspace checked out alongside this repository
- Local Zephyr branch `audio-board` with MSPM0L110x GPIO support merged
- `bb-imager-rs` with `flash zepto` support and GPIO-assisted BSL entry

#### Build the firmware image

```console
REPO_ROOT="$(git rev-parse --show-toplevel)"
ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb \
GNUARMEMB_TOOLCHAIN_PATH=/usr \
west build -p always \
  -d "${REPO_ROOT}/build/mspm0-zephyr" \
  -b mikrobus_hat \
  "${REPO_ROOT}/firmware/mspm0-gpo-extender/app" \
  -- \
  -G'Unix Makefiles' \
  -DBOARD_ROOT="${REPO_ROOT}/firmware"
```

Artifacts:

- `build/mspm0-zephyr/zephyr/zephyr.hex`
- `build/mspm0-zephyr/zephyr/zephyr.bin`

#### Flash the image

Current working command:

```console
REPO_ROOT="$(git rev-parse --show-toplevel)"
BB_IMAGER_CLI="${BB_IMAGER_CLI:-../bb-imager-rs/target/debug/bb-imager-cli}"
"${BB_IMAGER_CLI}" --verbose flash zepto \
  "${REPO_ROOT}/build/mspm0-zephyr/zephyr/zephyr.hex" \
  --reset-gpio GPIO24 \
  --bsl-gpio GPIO25 \
  /dev/hat/mcu_i2c0
```

Notes:

- `GPIO24` is `MCU_RESET`
- `GPIO25` is `MCU_BOOTLOADER_SEL`
- `/dev/hat/mcu_i2c0` is the working host-visible MSPM0 I2C/BSL bus
- The repo-built `bb-imager-cli` supports `flash zepto`; a stale installed CLI may not
- Flash verify over this transport is still not fully trustworthy; `--no-verify`
  may be required if the CRC stage reports a false failure after program/write succeeds

#### What is verified on hardware

- Zephyr GPIO works on this board with the local `audio-board` Zephyr branch
- `PA19` / `PA20` were driven successfully and observed at `J6`
- The mux is controllable from the MSPM0:
  - with `AN_SEL=0` and `INT_SEL=0`, an `AN -> INT` short routed to host `GPIO13` and `GPIO16`
  - with `AN_SEL=1` and `INT_SEL=1`, the same short routed to host `GPIO20` and `GPIO21`
  - with `AN_SEL=0` and `PWM_SEL=0`, an `AN -> PWM` short routed to host `GPIO17`
  - with `AN_SEL=0` and `PWM_SEL=1`, the same short routed to host `GPIO18`
  - with `AN_SEL=0`, the `AN -> MISO` combinations measured as:
    - `CIPO_SEL_0=0`, `CIPO_SEL_1=0` -> no host path selected
    - `CIPO_SEL_0=0`, `CIPO_SEL_1=1` -> host `GPIO20`
    - `CIPO_SEL_0=1`, `CIPO_SEL_1=0` -> host `GPIO16`
    - `CIPO_SEL_0=1`, `CIPO_SEL_1=1` -> host `GPIO17`

That proves:

- the updated flash path works
- the updated Zephyr MSPM0L1105 GPIO driver works
- the `AN_SEL`, `INT_SEL`, `PWM_SEL`, `CIPO_SEL_0`, and `CIPO_SEL_1`
  selectors are controllable in firmware
- `RST_SEL` can be validated from HAT pin `7` (`GPIO4`), which is tied to the
  `J7` signal through `R23` (`0 ohm`)

#### I2C target validation

The current `app/` image emulates a `pca9538`-compatible 8-bit GPIO expander
at address `0x20`.

Validated on hardware:

- `0x20` ACKs in normal boot
- `0x48` disappears after returning `GPIO25` low and resetting with `GPIO24`
- direct register access works:

```console
i2ctransfer -y 1 w1@0x20 0x00 r4
# returns: INPUT OUTPUT POLARITY CONFIG
# example: 0xc0 0xff 0x00 0xff
```

- a standard Linux `gpio-pca953x` client also works:

```console
echo pca9538 0x20 | sudo tee /sys/bus/i2c/devices/i2c-1/new_device
gpiodetect
gpioinfo -c gpiochip3
echo 0x20 | sudo tee /sys/bus/i2c/devices/i2c-1/delete_device
```

- with `gpio-pca953x` bound, expander bits 2 and 3 control the measured mux
  route:
  - `2=0, 3=0` gives the `GPIO13 -> GPIO16` path
  - `2=1, 3=1` gives the `GPIO20 -> GPIO21` path

That is the current proof point that MSPM0 I2C target mode and PCA9538-style
host control are both working on hardware.

#### Current validation recipe

The validation flow is now scripted:

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-int
```

With the audio board removed and `AN` shorted to `INT` on the mikroBUS socket,
that command produces:

```console
AN->INT low state (expect GPIO16=0 GPIO21=1): 0 1
AN->INT high state (expect GPIO16=1 GPIO21=0): 1 0
```

Manual hold-and-probe mode is also available:

1. Hold the host-side source levels apart:

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh hold an-int low
```

Expected result:

- probe or read `GPIO16` low
- probe or read `GPIO21` high

Then:

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh hold an-int high
```

Expected result:

- probe or read `GPIO16` high
- probe or read `GPIO21` low

- The script uses line names automatically when the host-side overlay is active.
- Without the overlay, it falls back to `gpiochip3` offsets for the expander.

#### Additional jumper setups for the remaining selectors

These setups are enough for host-side automated validation without probing the
MSPM0 pins directly:

- `AN -> PWM`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-pwm
```

Current measured output:

```console
AN->PWM low state (expect GPIO18=1 GPIO17=0): 1 0
AN->PWM high state (expect GPIO18=0 GPIO17=1): 0 1
```

- `AN -> MISO`

```console
./firmware/host-integration/linux/beagley-ai/validate-mux.sh sample an-miso
```

Current measured output:

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

Expected output:

```console
AN->RST low state (expect GPIO19=0 GPIO4=1): 0 1
AN->RST high state (expect GPIO19=1 GPIO4=0): 1 0
```

Probe or read HAT pin `35` (`GPIO19`) and HAT pin `7` (`GPIO4`). HAT pin `7`
is tied to the same signal brought out at `J7` through `R23` (`0 ohm`), so it
is the preferred validation point on this PCB revision.

#### Current gaps before production firmware

- `RST_SEL` still needs a measured validation run using HAT pin `7` (`GPIO4`)
- The current `mspm0-gpo-extender` app only exposes the mux selectors through
  the PCA9538 register model; host-profile policy is still future work
