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

### MSPM0 fixed mux image programming

This section captures the exact steps used on the BeagleY-AI host to build and
flash an MSPM0 image that drives the mux GPIOs to the documented
`BYAI-AM67A` defaults.

#### Intended mux state

Per `README.md`, the fixed profile is:

- `PA3=0`
- `PA4=0`
- `PA9=0`
- `PA10=0`
- `PA11=1`
- `PA15=0`

#### Host setup used

- Debian Bookworm on BeagleY-AI
- Rust installed with `rustup`
- `gcc-arm-none-eabi` and `binutils-arm-none-eabi` installed from Debian
- Python venv at `/home/beagle/.venvs/zephyr-tools` with `west`, `ninja`, and Zephyr Python requirements
- Local Zephyr workspace rooted at `/home/beagle` with Zephyr checked out at `/home/beagle/zephyr`

#### Build the firmware image

The working image is the direct-DriverLib GPIO implementation in
`firmware/mspm0-gpo-extender/app/src/main.c`.

```console
export PATH="/home/beagle/.venvs/zephyr-tools/bin:$PATH"
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=/usr

west build -p always \
  -b jkembedded_mikrobus_hat_mspm0 \
  /home/beagle/jkembedded-audio-board/firmware/mspm0-gpo-extender/app \
  --build-dir /home/beagle/jkembedded-audio-board/build/mspm0-zephyr \
  -- \
  -DBOARD_ROOT=/home/beagle/jkembedded-audio-board/firmware/mspm0-gpo-extender
```

Artifacts:

- `build/mspm0-zephyr/zephyr/zephyr.hex`
- `build/mspm0-zephyr/zephyr/zephyr.bin`

#### Build the host flasher

```console
cd /home/beagle/bb-imager-rs
. "$HOME/.cargo/env"
cargo build -p bb-imager-cli --no-default-features --features zepto_i2c
```

#### Enter MSPM0 ROM BSL

BeagleY-AI GPIO mapping from `firmware/host-programmers/README.md`:

- `MCU_RESET` -> `gpiochip1` line `10`
- `MCU_BOOTLOADER_SEL` -> `gpiochip2` line `42`

Hold bootloader select high in one shell:

```console
gpioset -c gpiochip2 -C mspm0-boot 42=1
```

Pulse reset in another shell:

```console
gpioset -c gpiochip1 -t 200ms,0 -C mspm0-reset 10=0
```

#### Flash the image

```console
. "$HOME/.cargo/env"
/home/beagle/bb-imager-rs/target/debug/bb-imager-cli flash zepto --no-verify \
  /home/beagle/jkembedded-audio-board/build/mspm0-zephyr/zephyr/zephyr.hex \
  /dev/i2c-1
```

Observed CLI status on the successful run:

- `[1] Preparing`
- `[3] Verifying`

#### Return the MSPM0 to normal boot

Release the `gpioset` process holding `MCU_BOOTLOADER_SEL`, then pulse reset
with bootloader select low:

```console
gpioset -c gpiochip2 -C mspm0-boot-low 42=0
gpioset -c gpiochip1 -t 200ms,0 -C mspm0-reset-normal 10=0
```

Final observed line state after release:

- `GPIO25=0`
- `GPIO24=1`

#### Verification status and next steps

What was verified in software:

- The Zephyr image built successfully.
- The host flasher completed without error using the patched ACK-only path.
- After the final normal-boot reset, `i2cdetect -y -r 1` no longer showed `0x48`;
  the remaining visible devices were `0x47`, `0x50`, and `0x60`.

What is still not directly verified from Linux:

- The six MSPM0 mux GPIO output levels were not read back electrically.
- The application does not yet expose the planned PCA9538-compatible I2C target
  at `0x20`.

Recommended next verification steps on hardware:

- Probe the mux select LEDs or test points and confirm:
  - `RST/WRD=0`
  - `PWM/BIT=0`
  - `AN/DI=0`
  - `INT/DO=0`
  - `CIPO_CNT_SEL0=1`
  - `CIPO_CNT_SEL1=0`
- Measure continuity or signal routing on the HAT connector to confirm the
  board is in the `BYAI-AM67A` path documented in `README.md`.
- Add a trivial observable application behavior for future flashing tests:
  - expose the planned PCA9538-compatible target at `0x20`, or
  - blink one mux LED in a recognizable pattern before settling to defaults.
- Revisit the Zephyr baseline: MSPM0 support moved forward after Zephyr 4.3, so
  update `west.yml` before implementing the full GPIO-expander firmware.
