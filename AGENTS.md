# Repository Guidelines

This repository now covers the hardware design and the accompanying firmware, host utilities, and documentation required to validate and program the board.

## Scope and conventions
- Keep the KiCad design files at the repository root; place all firmware in `firmware/`, host-side utilities in `firmware/host-programmers/`, and documentation under `docs/`.
- Prefer Zephyr for the MSPM0L1105TRGER GPO-extender firmware; the TI SDK is acceptable if Zephyr is impractical.
- Host-side programmers must be self-contained Rust or Crystal binaries that are statically linked for aarch64 Linux. Avoid Python and external PyPI dependencies.
- When adding host integrations, prioritize mainline-able device-tree overlays and minimal driver patches.

## CI expectations
- Update both `.gitlab-ci.yml` and `.github/workflows/` when adding new build or verification steps.
- Extend `ci/check-project-structure.sh` if the repository layout changes.

## Documentation
- Keep `README.md` aligned with the current directory layout and firmware/utility plans.
- Place deeper design notes in `docs/` and link them from `README.md` when relevant.
