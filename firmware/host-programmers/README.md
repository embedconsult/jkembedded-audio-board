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

## Planned features
- Board detection and safe default mux selection.
- BSL entry via BOOTLOADER_SEL and RESET control.
- Firmware image flashing and verification over I2C.
- Optional dry-run mode for validation during production testing.

## BeagleY-AI (current target) resource mapping

### Image used for testing

```
$ cat /etc/dogtag
BeagleBoard.org Debian Bookworm Xfce Image 2024-09-04
```

### HAT I2C

/dev/i2c-1

### HAT GPIO

| Signal             | pin | linename | chip | line |
| ------------------ | --- | -------- | ---- | ---- |
| MCU_RESET          | 18  | GPIO24   | 0    | 10   |
| MCU_BOOTLOADER_SEL | 22  | GPIO25   | 1    | 42   |
