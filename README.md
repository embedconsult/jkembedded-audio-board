# JK-Embedded Audio Board

## Tasks

- [X] A. Requirement analysis (2 days)
- [ ] B. Schematic entry (1 weeks after A approval)
- [ ] C. Layout (1 weeks after B approval)
- [ ] D. Prototype coordination (3 days after C approval)
- [ ] E. Test code development (2 weeks after C approval)
- [ ] F. Test bench materials (2 weeks after C approval)
- [ ] E. Prototype delivery (3 weeks after C approval)
- [ ] F. Sign-off on production design
- [ ] G. First production units ship (12 weeks after F approval)

## Requirements

1. Compatible with RPi HAT connector
2. Texas Instruments TAC5112 for audio codec
3. Texas Instruments DAC53002 for clock generator VCXO control
4. Silicon Laboratories Si5351 clock generator with VCXO (Si5351B)
5. SBC signals:
   * AFSX - left/right frame sync (SBC -> audio board)
   * ACLKX - bit clock (SBC -> audio board)
   * AXR0 - data I/O (TBD)
   * AXR1 - data I/O (TBD)
   * EQEP\_A - clock counting (audio board -> SBC)
   * CPTSx\_HWnTSPUSH or GPIO - timestamp event (audio board -> SBC)
   * SDA - I<sup>2</sup>C data I/O
   * SCL - I<sup>2</sup>C clock I/O
6. Audio signals:
   * Jacks for stereo in and out at line level (1V<sub>RMS</sub> @ 100Ω) or better
   * Additional I/Os over 3-pin headers
7. Texas Instruments MSPM0C1103 for header signal mapping

## HAT signal mapping

| SBC signal | BP-AM62    | BYAI-AM67A | SK-AM62   | SK-AM68/9 |
| ---------- | ---------- | ---------- | --------- | --------- |
| AFSX       | RST (D20)  | 33 (E19)   | N/A (D20) | 35        |
| ACLKX      | PWM (B20)  | 12 (D25)   | 11 (B20)  | 12        |
| AXR0       | AN (E18)   | 38 (F23)   | 33 (E18)  | 38        |
| AXR1       | INT (B18)  | 40 (B25)   | 36 (B18)  | 40        |
| EQEP\_A    | CIPO (B19) | 36 (A25)   | 38 (B19)  | 11        |
| HWxTSPUSH  | COPI (A19) | 19 (B12)   | 19 (B13)  | 19        |
| SDA        | SDA (B15)  | 3 (E11)    | 3 (K24)   | 3         |
| SCL        | SCL (A15)  | 5 (B13)    | 5 (K22)   | 5         |
| 3.3V       | +3.3V      | 1,17       | 1,17      | 1,17      |
| 5.0V       | +5V        | 2,4        | 2,4       | 2,4       |
| GND        | GND        | GND        | GND       | GND       |
| tbd        | RX (C15)   | 10 (C27)   | 10 (C15)  |
| tbd        | TX (E15)   | 8 (F24)    | 8 (E15)   |
| tbd        | SCK (A20)  | 23 (A9)    | 23 (A14)  |
| tbd        | CS (E19)   | 24 (C12)   | 24 (A13)  |

