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

