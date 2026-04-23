# JK-Embedded Audio Board

KiCad design files, firmware plans, and host-side utilities for the JK-Embedded Audio Board and its mikroBUS HAT companion. The design targets multiple SBC hosts (BeaglePlay, BeagleY-AI, TI AM62/68/69 SK-EVM) and uses an MSPM0L1105TRGER as a GPO-extender for configuring the analog muxes.

## Tasks

- [X] A. Requirement analysis (2 days)
- [X] B. Schematic entry (1 weeks after A approval)
- [X] C. Layout (1 weeks after B approval)
- [X] D. Prototype coordination (3 days after C approval)
- [ ] E. Test code development (2 weeks after C approval)
- [ ] F. Test bench materials (2 weeks after C approval)
- [ ] G. Prototype delivery (3 weeks after C approval)
- [ ] H. Sign-off on production design
- [ ] I. First production units ship (12 weeks after H approval)

## Project scope
- **Hardware**: KiCad schematics/PCB for the audio board, mikroBUS HAT, and reference bring-up materials.
- **Firmware**: MSPM0 GPO-extender for mux control (Zephyr preferred; TI SDK acceptable) plus host-side programming utilities and sample integrations for Linux/Zephyr-based hosts.
- **Host integration**: Device-tree overlays and minimal driver/configuration routines for the Si5351 clock generator and the audio codec. Aim to upstream DTS fragments where possible.
- **Production/test utilities**: Board programmers and validation helpers for all target hosts except BeaglePlay (which does not use the mikroBUS HAT).

## Repository layout
- `AGENTS.md` — contribution expectations (scope, CI, documentation rules).
- `docs/` — high-level design notes, firmware/utility plans, and host integration checklists.
- `firmware/` — all firmware and host-side utilities.
  - `mspm0-gpo-extender/` — Zephyr/TI SDK project for the MSPM0L1105TRGER mux controller.
  - `host-programmers/` — Rust/Crystal utilities to program the MSPM0 over I2C/BSL from aarch64 Linux hosts.
  - `host-integration/` — device-tree overlays, clock generator configuration helpers, and sample host code.
- `ci/` — shared CI scripts (e.g., structure checks) used by GitLab and GitHub workflows.
- KiCad sources remain at the repository root for the board and HAT designs.

## Requirements

1. Compatible with RPi HAT connector
2. Texas Instruments TAC5112 for audio codec
3. Texas Instruments DAC53002 for clock generator VCXO control
4. Silicon Laboratories Si5351 clock generator with VCXO (Si5351B)
5. SBC signals:
   * AFSX (WRD) - left/right frame sync (audio board -> SBC)
   * ACLKX (BIT) - bit clock (audio board -> SBC)
   * AXR0 (DI) - data in (SBC -> audio board)
   * AXR1 (DO) - data out (audio board -> SBC)
   * EQEP_A (CNT) - clock counting (audio board -> SBC)
   * CPTSx_HWnTSPUSH or GPIO (TS) - timestamp event (audio board -> SBC)
   * SDA - I<sup>2</sup>C data I/O
   * SCL - I<sup>2</sup>C clock I/O
6. Audio signals:
   * Jacks for stereo in and out at line level (1V<sub>RMS</sub> @ 100Ω) or better
   * Additional I/Os over 3-pin headers
7. Texas Instruments MSPM0L1105TRGER for header signal mapping

## MSPM0 GPO-extender goals
- Mimic a simple I2C GPO-expander interface for host control while optionally auto-selecting mux states per detected host.
- Use BOOTLOADER_SEL and RESET signals to enter the MSPM0 ROM BSL for programming via I2C.
- Keep the binary footprint small and configuration-driven to ease host integration.

## Current validated state
- The local `zephyr` `audio-board` branch includes working MSPM0L110x GPIO support for this board.
- The shared Zephyr board definition for MSPM0 firmware now lives at `firmware/boards/arm/mikrobus_hat/`.
- The BeagleY-AI flash path is working with:
  - `GPIO24` as `MCU_RESET`
  - `GPIO25` as `MCU_BOOTLOADER_SEL`
  - `/dev/hat/mcu_i2c0` as the MSPM0 I2C/BSL bus
  - `bb-imager-cli --verbose flash zepto ... --reset-gpio GPIO24 --bsl-gpio GPIO25 /dev/hat/mcu_i2c0`
- `firmware/blinky/` is the clean minimal Zephyr GPIO validation app and is a better board/GPIO smoke test than the earlier ad hoc debug images.
- The MSPM0 now emulates a `pca9538`-compatible GPIO expander at `0x20`, and Linux can bind the standard `gpio-pca953x` driver to it.
- Zephyr GPIO control has been verified electrically on:
  - `PA19` / `PA20` at `J6`
  - `PA9` / `PA10` through the muxes using an `AN -> INT` short on the mikroBUS socket

## Host programmer expectations
- Produce statically linked aarch64 binaries (Rust or Crystal) with no runtime dependencies beyond the Linux kernel.
- Discover GPIO line names and I2C controller symlinks when available; otherwise fall back to board-specific static mappings or CLI arguments.
- Include board profiles for BeagleY-AI, TI AM62 SK-EVM, TI AM68 SK-EVM, and TI AM69 SK-EVM; BeaglePlay is excluded from programming because it does not use the mikroBUS HAT.

## CI overview
- **GitLab CI**: Generates PDFs, BOMs, fabrication outputs, renders, and packaging assets from the KiCad sources, plus layout sanity checks. See `.gitlab-ci.yml`.
- **GitHub Actions**: Mirrors the layout checks and KiCad artifact generation for GitHub users. See `.github/workflows/ci.yml`.
- Shared CI logic lives in `ci/check-project-structure.sh`. Update both pipelines when adding new build steps.

## HAT signal mapping

Where do the various SBC signals need to go to the board connector for each board?

| SBC signal | BP-AM62    | BYAI-AM67A | SK-AM62   | SK-AM68/9 |
| ---------- | ---------- | ---------- | --------- | --------- |
| WRD        | RST (D20)  | 35 (C26)   | N/A (D20) | 35        |
| BIT        | PWM (B20)  | 12 (D25)   | 11 (B20)  | 12        |
| DI         | AN (E18)   | 38 (F23)   | 33 (E18)  | 38        |
| DO         | INT (B18)  | 40 (B25)   | 36 (B18)  | 40        |
| CNT        | CIPO (B19) | 36 (A25)   | 38 (B19)  | 11        |
| TS         | COPI (A19) | 19 (B12)   | 19 (B13)  | 19        |
| SDA        | SDA (B15)  | 3 (E11)    | 3 (K24)   | 3         |
| SCL        | SCL (A15)  | 5 (B13)    | 5 (K22)   | 5         |
| 3.3V       | +3.3V      | 1,17       | 1,17      | 1,17      |
| 5.0V       | +5V        | 2,4        | 2,4       | 2,4       |
| GND        | GND        | GND        | GND       | GND       |
| tbd        | RX (C15)   | 10 (C27)   | 10 (C15)  | 10        |
| tbd        | TX (E15)   | 8 (F24)    | 8 (E15)   | 8         |
| tbd        | SCK (A20)  | 23 (A9)    | 23 (A14)  | 23        |
| SDQ        | CS (E19)   | 24 (C12)   | 24 (A13)  | 24        |

## GPIO switches

The MSPM0 firmware exposes these selector lines through the emulated
`pca9538` GPIO expander:

| PCA9538 bit | Selector line | MSPM0 pin | Controlled route |
| ----------- | ------------- | --------- | ---------------- |
| 0           | `RST_SEL`     | `PA3`     | `WRD` between HAT pin `35` and `J7/7` |
| 1           | `PWM_SEL`     | `PA4`     | `BIT` between HAT pin `11` and HAT pin `12` |
| 2           | `AN_SEL`      | `PA9`     | `DI` between HAT pin `33` and HAT pin `38` |
| 3           | `INT_SEL`     | `PA10`    | `DO` between HAT pin `36` and HAT pin `40` |
| 4           | `CIPO_SEL_0`  | `PA11`    | `CNT` / `CIPO` selector bit 0 |
| 5           | `CIPO_SEL_1`  | `PA15`    | `CNT` / `CIPO` selector bit 1 |

Single-bit selector settings:

| Selector line | `0` | `1` |
| ------------- | --- | --- |
| `RST_SEL`     | `WRD` on HAT pin `35` | `WRD` on `J7/7` |
| `PWM_SEL`     | `BIT` on HAT pin `11` | `BIT` on HAT pin `12` |
| `AN_SEL`      | `DI` on HAT pin `33` | `DI` on HAT pin `38` |
| `INT_SEL`     | `DO` on HAT pin `36` | `DO` on HAT pin `40` |

Two-bit `CNT` / `CIPO` selector settings:

| `CIPO_SEL_0` | `CIPO_SEL_1` | `CNT` on |
| ------------ | ------------ | -------- |
| `0`          | `0`          | not connected |
| `0`          | `1`          | HAT pin `38` |
| `1`          | `0`          | HAT pin `36` |
| `1`          | `1`          | HAT pin `11` |

Selector settings for each target host:

| Target host | `RST_SEL` | `PWM_SEL` | `AN_SEL` | `INT_SEL` | `CIPO_SEL_0` | `CIPO_SEL_1` |
| ----------- | --------- | --------- | -------- | --------- | ------------ | ------------ |
| `BYAI-AM67A` | `0` | `1` | `1` | `1` | `1` | `0` |
| `SK-AM62`    | `1` | `0` | `0` | `0` | `0` | `1` |
| `SK-AM68/9`  | `0` | `1` | `1` | `1` | `1` | `1` |

## Next steps
- Finish `RST_SEL` validation after fixing or bypassing the undersized `J7` connector on this PCB revision.
- Decide the production MSPM0 firmware interface:
  - fixed per-host mux profiles, or
  - an I2C target / GPO-expander compatible interface.
- Keep `firmware/blinky/` as the basic board/GPIO validation sample.
- Add device-tree overlays and minimal driver hooks in `firmware/host-integration/` for clock generator initialization and codec wiring.
- Integrate firmware and utility builds into both CI systems.
