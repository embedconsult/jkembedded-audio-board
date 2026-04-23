#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c/target/eeprom.h>

static const struct device *eeprom = DEVICE_DT_GET(DT_NODELABEL(eeprom0));

int main(void)
{
	if (!device_is_ready(eeprom)) {
		return 0;
	}

	if (i2c_target_driver_register(eeprom) < 0) {
		return 0;
	}

	k_sleep(K_FOREVER);
	return 0;
}
