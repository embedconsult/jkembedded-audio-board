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
