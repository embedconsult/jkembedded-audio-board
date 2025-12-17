# JK-Embedded Audio Board

KiCad design files, firmware plans, and host-side utilities for the JK-Embedded Audio Board and its mikroBUS HAT companion. The design targets multiple SBC hosts (BeaglePlay, BeagleY-AI, TI AM62/68/69 SK-EVM) and uses an MSPM0L1105TRGER as a GPO-extender for configuring the analog muxes.

## Tasks

- [X] A. Requirement analysis (2 days)
- [X] B. Schematic entry (1 weeks after A approval)
- [X] C. Layout (1 weeks after B approval)
- [ ] D. Prototype coordination (3 days after C approval)
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
7. Texas Instruments MSPM0C1103 for header signal mapping

## MSPM0 GPO-extender goals
- Mimic a simple I2C GPO-expander interface for host control while optionally auto-selecting mux states per detected host.
- Use BOOTLOADER_SEL and RESET signals to enter the MSPM0 ROM BSL for programming via I2C.
- Keep the binary footprint small and configuration-driven to ease host integration.

## Host programmer expectations
- Produce statically linked aarch64 binaries (Rust or Crystal) with no runtime dependencies beyond the Linux kernel.
- Discover GPIO line names and I2C controller symlinks when available; otherwise fall back to board-specific static mappings or CLI arguments.
- Include board profiles for BeagleY-AI, TI AM62 SK-EVM, TI AM68 SK-EVM, and TI AM69 SK-EVM; BeaglePlay is excluded from programming because it does not use the mikroBUS HAT.

## CI overview
- **GitLab CI**: Generates PDFs, BOMs, fabrication outputs, renders, and packaging assets from the KiCad sources, plus layout sanity checks. See `.gitlab-ci.yml`.
- **GitHub Actions**: Mirrors the layout checks and KiCad artifact generation for GitHub users. See `.github/workflows/ci.yml`.
- Shared CI logic lives in `ci/check-project-structure.sh`. Update both pipelines when adding new build steps.

## HAT signal mapping

| SBC signal | BP-AM62    | BYAI-AM67A | SK-AM62   | SK-AM68/9 |
| ---------- | ---------- | ---------- | --------- | --------- |
| WRD        | RST (D20)  | 35 (C26)   | N/A (D20) | 35        |
| BIT        | PWM (B20)  | 12 (D25)   | 11 (B20)  | 12        |
| DI         | AN (E18)   | 38 (F23)   | 33 (E18)  | 38        |
| DO         | INT (B18)  | 40 (B25)   | 36 (B18)  | 40        |
| CNT        | POCI (B19) | 36 (A25)   | 38 (B19)  | 11        |
| TS         | PICO (A19) | 19 (B12)   | 19 (B13)  | 19        |
| SDA        | SDA (B15)  | 3 (E11)    | 3 (K24)   | 3         |
| SCL        | SCL (A15)  | 5 (B13)    | 5 (K22)   | 5         |
| 3.3V       | +3.3V      | 1,17       | 1,17      | 1,17      |
| 5.0V       | +5V        | 2,4        | 2,4       | 2,4       |
| GND        | GND        | GND        | GND       | GND       |
| tbd        | RX (C15)   | 10 (C27)   | 10 (C15)  | 10        |
| tbd        | TX (E15)   | 8 (F24)    | 8 (E15)   | 8         |
| tbd        | SCK (A20)  | 23 (A9)    | 23 (A14)  | 23        |
| SDQ        | CS (E19)   | 24 (C12)   | 24 (A13)  | 24        |

## Next steps
- Finalize the MSPM0 Zephyr/TI SDK project in `firmware/mspm0-gpo-extender/`.
- Implement the host programmer utility and board detection logic in `firmware/host-programmers/`.
- Add device-tree overlays and minimal driver hooks in `firmware/host-integration/` for clock generator initialization and codec wiring.
- Integrate firmware and utility builds into both CI systems.
