#!/bin/bash -e
# GPIO24 = MSPM0 Reset
# GPIO25 = BSL Invoke
BSLI_PID=0
RESET_PID=0
function cleanup {
	if [ $RESET_PID -ne 0 ]; then
		echo "Releasing RESET pin"
		kill $RESET_PID
		RESET_PID=0
	fi
	sleep 0.1
	if [ $BSLI_PID -ne 0 ]; then
		echo "Releasing BSL Invoke pin"
		kill $BSLI_PID
		BSLI_PID=0
	fi
}
trap cleanup EXIT

echo "Setting BSL Invoke pin high"
gpioset -C bsl-flasher --by-name GPIO25=1 &
BSLI_PID=$!

echo "Setting RESET pin low"
gpioset -C bsl-flasher --by-name GPIO24=0 &
RESET_PID=$!

sleep 0.1

cleanup

bb-imager-cli flash zepto build/zephyr/zephyr.bin /dev/hat/mcu_i2c0
