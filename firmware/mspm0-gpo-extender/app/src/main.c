#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define USER_NODE DT_PATH(zephyr_user)

static const struct gpio_dt_spec debug_pa19 =
	GPIO_DT_SPEC_GET(USER_NODE, debug_pa19_gpios);
static const struct gpio_dt_spec debug_pa20 =
	GPIO_DT_SPEC_GET(USER_NODE, debug_pa20_gpios);

static int configure_debug_gpio(void)
{
	if (!gpio_is_ready_dt(&debug_pa19) || !gpio_is_ready_dt(&debug_pa20)) {
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&debug_pa19, GPIO_OUTPUT_INACTIVE) < 0) {
		return -EIO;
	}

	if (gpio_pin_configure_dt(&debug_pa20, GPIO_OUTPUT_ACTIVE) < 0) {
		return -EIO;
	}

	return 0;
}

int main(void)
{
	int ret = configure_debug_gpio();

	if (ret < 0) {
		return ret;
	}

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
