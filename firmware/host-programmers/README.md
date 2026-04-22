# Host Programmer Utilities

Utilities in this directory program the MSPM0L1105TRGER mux-control firmware from supported hosts.

## Requirements
- Rust or Crystal implementation only; produce statically linked aarch64 Linux binaries.
- No Python or PyPI dependencies.
- Use GPIO line names and I2C controller symlinks when available; allow CLI overrides and static board profiles otherwise.

## Target hosts
- BeagleY-AI
- TI AM62 SK-EVM
- TI AM68 SK-EVM
- TI AM69 SK-EVM

BeaglePlay is excluded because it does not use the mikroBUS HAT.

## Current status
- GPIO line-name based BSL entry is working on the BeagleY-AI host
- `bb-imager-cli flash zepto` is the current working flash path
- The working invocation is:

```console
/home/beagle/bb-imager-rs/target/debug/bb-imager-cli --verbose flash zepto \
  /path/to/zephyr.hex \
  --reset-gpio GPIO24 \
  --bsl-gpio GPIO25 \
  /dev/hat/mcu_i2c0
```

- Flash verify may still false-fail on this transport; use `--no-verify` if the
  image is written successfully but the CRC stage reports an error

## Planned features
- Board detection and safe default mux selection
- More robust verification/readback over I2C BSL
- Optional dry-run mode for validation during production testing

## BeagleY-AI (current target) resource mapping

### Image used for testing

```
$ cat /etc/dogtag
BeagleBoard.org Debian Bookworm Xfce Image 2025-11-25
```

### HAT I2C

Preferred path:

`/dev/hat/mcu_i2c0`

Resolved bus on this host:

`/dev/i2c-1`

```
beagle@beagle:~/jkembedded-audio-board$ i2cdetect -y -r 1
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

### HAT GPIO

| Signal             | pin | linename |
| ------------------ | --- | -------- |
| MCU_RESET          | 18  | GPIO24   |
| MCU_BOOTLOADER_SEL | 22  | GPIO25   |
