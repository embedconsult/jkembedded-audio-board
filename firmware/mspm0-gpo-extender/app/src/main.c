#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define USER_NODE DT_PATH(zephyr_user)

static const struct gpio_dt_spec rst_wrd_sel =
	GPIO_DT_SPEC_GET(USER_NODE, rst_wrd_sel_gpios);
static const struct gpio_dt_spec pwm_bit_sel =
	GPIO_DT_SPEC_GET(USER_NODE, pwm_bit_sel_gpios);
static const struct gpio_dt_spec an_di_sel =
	GPIO_DT_SPEC_GET(USER_NODE, an_di_sel_gpios);
static const struct gpio_dt_spec int_do_sel =
	GPIO_DT_SPEC_GET(USER_NODE, int_do_sel_gpios);
static const struct gpio_dt_spec cipo_cnt_sel0 =
	GPIO_DT_SPEC_GET(USER_NODE, cipo_cnt_sel0_gpios);
static const struct gpio_dt_spec cipo_cnt_sel1 =
	GPIO_DT_SPEC_GET(USER_NODE, cipo_cnt_sel1_gpios);
static const struct gpio_dt_spec bootloader_sel =
	GPIO_DT_SPEC_GET(USER_NODE, bootloader_sel_gpios);

static int configure_mux_pin(const struct gpio_dt_spec *spec, int value)
{
	if (!gpio_is_ready_dt(spec)) {
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(spec, value ? GPIO_OUTPUT_ACTIVE : GPIO_OUTPUT_INACTIVE) < 0) {
		return -EIO;
	}

	return 0;
}

static int configure_mux_state(void)
{
	if (configure_mux_pin(&rst_wrd_sel, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&pwm_bit_sel, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&an_di_sel, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&int_do_sel, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&cipo_cnt_sel0, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&cipo_cnt_sel1, 0) < 0) {
		return -EIO;
	}

	if (configure_mux_pin(&bootloader_sel, 0) < 0) {
		return -EIO;
	}

	return 0;
}

int main(void)
{
	int ret = configure_mux_state();

	if (ret < 0) {
		return ret;
	}

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
