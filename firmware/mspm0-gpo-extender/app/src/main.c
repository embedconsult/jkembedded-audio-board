#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#define USER_NODE DT_PATH(zephyr_user)

#define PCA9538_I2C_ADDR      0x20
#define PCA9538_REG_INPUT     0x00
#define PCA9538_REG_OUTPUT    0x01
#define PCA9538_REG_POLARITY  0x02
#define PCA9538_REG_CONFIG    0x03
#define PCA9538_REG_MASK      0x03
#define PCA9538_HW_GPIO_COUNT 6
#define PCA9538_HW_MASK       GENMASK(PCA9538_HW_GPIO_COUNT - 1, 0)

struct pca9538_state {
	uint8_t output;
	uint8_t polarity;
	uint8_t config;
	uint8_t reg_ptr;
	bool reg_ptr_valid;
};

static const struct gpio_dt_spec mux_gpios[PCA9538_HW_GPIO_COUNT] = {
	GPIO_DT_SPEC_GET(USER_NODE, rst_wrd_sel_gpios),
	GPIO_DT_SPEC_GET(USER_NODE, pwm_bit_sel_gpios),
	GPIO_DT_SPEC_GET(USER_NODE, an_di_sel_gpios),
	GPIO_DT_SPEC_GET(USER_NODE, int_do_sel_gpios),
	GPIO_DT_SPEC_GET(USER_NODE, cipo_cnt_sel0_gpios),
	GPIO_DT_SPEC_GET(USER_NODE, cipo_cnt_sel1_gpios),
};

static const struct device *const i2c_target = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static struct pca9538_state pca9538 = {
	.output = 0xffU,
	.polarity = 0x00U,
	.config = 0xffU,
	.reg_ptr = PCA9538_REG_INPUT,
	.reg_ptr_valid = false,
};

static int pca9538_apply_outputs(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(mux_gpios); i++) {
		const struct gpio_dt_spec *spec = &mux_gpios[i];
		gpio_flags_t flags;
		bool is_input;
		bool level;
		int ret;

		if (!gpio_is_ready_dt(spec)) {
			return -ENODEV;
		}

		is_input = (pca9538.config & BIT(i)) != 0U;
		level = (pca9538.output & BIT(i)) != 0U;
		flags = is_input ? GPIO_INPUT : (level ? GPIO_OUTPUT_ACTIVE : GPIO_OUTPUT_INACTIVE);

		ret = gpio_pin_configure_dt(spec, flags);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

static uint8_t pca9538_input_value(void)
{
	uint8_t value = pca9538.output & ~PCA9538_HW_MASK;

	for (size_t i = 0; i < ARRAY_SIZE(mux_gpios); i++) {
		int ret = gpio_pin_get_dt(&mux_gpios[i]);

		if (ret > 0) {
			value |= BIT(i);
		} else {
			value &= ~BIT(i);
		}
	}

	return value ^ pca9538.polarity;
}

static uint8_t pca9538_read_reg(uint8_t reg)
{
	switch (reg & PCA9538_REG_MASK) {
	case PCA9538_REG_INPUT:
		return pca9538_input_value();
	case PCA9538_REG_OUTPUT:
		return pca9538.output;
	case PCA9538_REG_POLARITY:
		return pca9538.polarity;
	case PCA9538_REG_CONFIG:
		return pca9538.config;
	default:
		return 0xffU;
	}
}

static void pca9538_write_reg(uint8_t reg, uint8_t value)
{
	switch (reg & PCA9538_REG_MASK) {
	case PCA9538_REG_OUTPUT:
		pca9538.output = value;
		(void)pca9538_apply_outputs();
		break;
	case PCA9538_REG_POLARITY:
		pca9538.polarity = value;
		break;
	case PCA9538_REG_CONFIG:
		pca9538.config = value;
		(void)pca9538_apply_outputs();
		break;
	default:
		break;
	}
}

static int pca9538_write_requested(struct i2c_target_config *cfg)
{
	ARG_UNUSED(cfg);

	pca9538.reg_ptr_valid = false;
	return 0;
}

static int pca9538_write_received(struct i2c_target_config *cfg, uint8_t value)
{
	ARG_UNUSED(cfg);

	if (!pca9538.reg_ptr_valid) {
		pca9538.reg_ptr = value & PCA9538_REG_MASK;
		pca9538.reg_ptr_valid = true;
		return 0;
	}

	pca9538_write_reg(pca9538.reg_ptr, value);
	pca9538.reg_ptr = (pca9538.reg_ptr + 1U) & PCA9538_REG_MASK;
	return 0;
}

static int pca9538_read_requested(struct i2c_target_config *cfg, uint8_t *value)
{
	ARG_UNUSED(cfg);

	if (!pca9538.reg_ptr_valid) {
		pca9538.reg_ptr = PCA9538_REG_INPUT;
		pca9538.reg_ptr_valid = true;
	}

	*value = pca9538_read_reg(pca9538.reg_ptr);
	return 0;
}

static int pca9538_read_processed(struct i2c_target_config *cfg, uint8_t *value)
{
	ARG_UNUSED(cfg);

	pca9538.reg_ptr = (pca9538.reg_ptr + 1U) & PCA9538_REG_MASK;
	*value = pca9538_read_reg(pca9538.reg_ptr);
	return 0;
}

static int pca9538_stop(struct i2c_target_config *cfg)
{
	ARG_UNUSED(cfg);
	return 0;
}

static const struct i2c_target_callbacks pca9538_callbacks = {
	.write_requested = pca9538_write_requested,
	.write_received = pca9538_write_received,
	.read_requested = pca9538_read_requested,
	.read_processed = pca9538_read_processed,
	.stop = pca9538_stop,
};

static struct i2c_target_config pca9538_target_cfg = {
	.address = PCA9538_I2C_ADDR,
	.callbacks = &pca9538_callbacks,
};

int main(void)
{
	if (!device_is_ready(i2c_target)) {
		return 0;
	}

	if (pca9538_apply_outputs() < 0) {
		return 0;
	}

	if (i2c_target_register(i2c_target, &pca9538_target_cfg) < 0) {
		return 0;
	}

	k_sleep(K_FOREVER);
	return 0;
}
