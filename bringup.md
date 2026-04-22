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
- Local Zephyr workspace rooted at `/home/beagle`
- Local Zephyr branch `audio-board` with MSPM0L110x GPIO support merged
- `bb-imager-rs` with `flash zepto` support and GPIO-assisted BSL entry

#### Build the firmware image

```console
/home/beagle/.venvs/zephyr-tools/bin/west build \
  -d /home/beagle/jkembedded-audio-board/build/mspm0-zephyr \
  -b jkembedded_mikrobus_hat_mspm0 \
  /home/beagle/jkembedded-audio-board/firmware/mspm0-gpo-extender/app
```

Artifacts:

- `build/mspm0-zephyr/zephyr/zephyr.hex`
- `build/mspm0-zephyr/zephyr/zephyr.bin`

#### Flash the image

Current working command:

```console
/home/beagle/bb-imager-rs/target/debug/bb-imager-cli --verbose flash zepto \
  /home/beagle/jkembedded-audio-board/build/mspm0-zephyr/zephyr/zephyr.hex \
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
  - with `PA9=0` and `PA10=0`, an `AN -> INT` short routed to host `GPIO13` and `GPIO16`
  - with `PA9=1` and `PA10=1`, the same short routed to host `GPIO20` and `GPIO21`

That proves:

- the updated flash path works
- the updated Zephyr MSPM0L1105 GPIO driver works
- at least the `AN_DI_SEL` and `INT_DO_SEL` mux selectors are controllable in firmware

#### Current gaps before production firmware

- `PA3`, `PA4`, `PA11`, and `PA15` still need the same measured-polarity validation
- The production MSPM0 firmware interface is still a decision:
  - fixed host-profile image, or
  - I2C target / GPO-expander style interface
- The current `mspm0-gpo-extender` app is a hardware-validation app, not final product firmware
