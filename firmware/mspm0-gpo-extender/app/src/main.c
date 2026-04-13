#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c/target/eeprom.h>
#include <zephyr/kernel.h>

static const struct device *const eeprom = DEVICE_DT_GET(DT_NODELABEL(eeprom0));

static const uint8_t pattern[] = {
	0x4d, 0x53, 0x50, 0x4d, 0x30, 0x20, 0x49, 0x32,
	0x43, 0x20, 0x4f, 0x4b, 0x20, 0x20, 0x20, 0x0a,
};

int main(void)
{
	if (!device_is_ready(eeprom)) {
		return 0;
	}

	(void)eeprom_target_program(eeprom, pattern, sizeof(pattern));

	if (i2c_target_driver_register(eeprom) != 0) {
		return 0;
	}

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
