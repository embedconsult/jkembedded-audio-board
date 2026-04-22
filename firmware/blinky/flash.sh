#!/bin/bash -e
RESET="GPIO24"
BSL="GPIO25"
bb-imager-cli --verbose flash zepto --no-verify build/zephyr/zephyr.hex --reset-gpio $RESET --bsl-gpio $BSL /dev/hat/mcu_i2c0

