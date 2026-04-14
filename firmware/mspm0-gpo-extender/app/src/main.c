#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include <ti/driverlib/m0p/dl_sysctl.h>

#define USER_NODE DT_PATH(zephyr_user)

static const struct gpio_dt_spec debug_pa19 =
	GPIO_DT_SPEC_GET(USER_NODE, debug_pa19_gpios);
static const struct gpio_dt_spec debug_pa20 =
	GPIO_DT_SPEC_GET(USER_NODE, debug_pa20_gpios);

static int configure_debug_gpio(void)
{
	int ret;

	DL_SYSCTL_disableSWD();

	if (!gpio_is_ready_dt(&debug_pa19) || !gpio_is_ready_dt(&debug_pa20)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&debug_pa19, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&debug_pa20, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return ret;
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
		gpio_pin_toggle_dt(&debug_pa19);
		gpio_pin_toggle_dt(&debug_pa20);
		k_busy_wait(100000);
	}

	return 0;
}
