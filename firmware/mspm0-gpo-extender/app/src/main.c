#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#define PCA9538_I2C_ADDR 0x20
#define PCA9538_REG_INPUT 0x00
#define PCA9538_REG_OUTPUT 0x01
#define PCA9538_REG_POLARITY 0x02
#define PCA9538_REG_CONFIG 0x03
#define PCA9538_REG_COUNT 4
#define PCA9538_GPIO_MASK 0x3f
#define PCA9538_RESERVED_MASK 0xc0

struct pca9538_state {
	uint8_t regs[PCA9538_REG_COUNT];
	uint8_t reg_ptr;
	bool reg_ptr_set;
};

static const struct gpio_dt_spec mux_gpios[] = {
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), rst_wrd_sel_gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), pwm_bit_sel_gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), an_di_sel_gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), int_do_sel_gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), cipo_cnt_sel0_gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(mux_pins), cipo_cnt_sel1_gpios),
};

static const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static struct pca9538_state pca_state;

static int pca9538_apply_gpio_config(const struct pca9538_state *state)
{
	for (size_t i = 0; i < ARRAY_SIZE(mux_gpios); i++) {
		uint8_t bit = BIT(i);
		bool is_input = (state->regs[PCA9538_REG_CONFIG] & bit) != 0U;
		int flags = is_input ? GPIO_INPUT : GPIO_OUTPUT_INACTIVE;

		if (!device_is_ready(mux_gpios[i].port)) {
			return -ENODEV;
		}

		int ret = gpio_pin_configure_dt(&mux_gpios[i], flags);
		if (ret != 0) {
			return ret;
		}

		if (!is_input) {
			ret = gpio_pin_set_dt(&mux_gpios[i],
						(state->regs[PCA9538_REG_OUTPUT] & bit) != 0U);
			if (ret != 0) {
				return ret;
			}
		}
	}

	return 0;
}

static uint8_t pca9538_read_input(const struct pca9538_state *state)
{
	uint8_t value = 0U;

	for (size_t i = 0; i < ARRAY_SIZE(mux_gpios); i++) {
		int ret = gpio_pin_get_dt(&mux_gpios[i]);
		if (ret > 0) {
			value |= BIT(i);
		}
	}

	value ^= (state->regs[PCA9538_REG_POLARITY] & PCA9538_GPIO_MASK);
	value &= PCA9538_GPIO_MASK;
	return value;
}

static uint8_t pca9538_get_register(const struct pca9538_state *state, uint8_t reg)
{
	switch (reg) {
	case PCA9538_REG_INPUT:
		return pca9538_read_input(state);
	case PCA9538_REG_OUTPUT:
	case PCA9538_REG_POLARITY:
	case PCA9538_REG_CONFIG:
		return state->regs[reg];
	default:
		return 0U;
	}
}

static void pca9538_write_register(struct pca9538_state *state, uint8_t reg,
				  uint8_t value)
{
	switch (reg) {
	case PCA9538_REG_OUTPUT:
		state->regs[PCA9538_REG_OUTPUT] &= ~PCA9538_GPIO_MASK;
		state->regs[PCA9538_REG_OUTPUT] |= (value & PCA9538_GPIO_MASK);
		(void)pca9538_apply_gpio_config(state);
		break;
	case PCA9538_REG_POLARITY:
		state->regs[PCA9538_REG_POLARITY] &= ~PCA9538_GPIO_MASK;
		state->regs[PCA9538_REG_POLARITY] |= (value & PCA9538_GPIO_MASK);
		break;
	case PCA9538_REG_CONFIG:
		state->regs[PCA9538_REG_CONFIG] = (value & PCA9538_GPIO_MASK) |
					    PCA9538_RESERVED_MASK;
		(void)pca9538_apply_gpio_config(state);
		break;
	default:
		break;
	}
}

static int pca9538_write_requested(struct i2c_target_config *cfg)
{
	ARG_UNUSED(cfg);
	pca_state.reg_ptr_set = false;
	return 0;
}

static int pca9538_write_received(struct i2c_target_config *cfg, uint8_t val)
{
	ARG_UNUSED(cfg);

	if (!pca_state.reg_ptr_set) {
		pca_state.reg_ptr = val % PCA9538_REG_COUNT;
		pca_state.reg_ptr_set = true;
		return 0;
	}

	pca9538_write_register(&pca_state, pca_state.reg_ptr, val);
	pca_state.reg_ptr = (pca_state.reg_ptr + 1U) % PCA9538_REG_COUNT;
	return 0;
}

static int pca9538_read_requested(struct i2c_target_config *cfg, uint8_t *val)
{
	ARG_UNUSED(cfg);

	if (!pca_state.reg_ptr_set) {
		pca_state.reg_ptr = PCA9538_REG_INPUT;
		pca_state.reg_ptr_set = true;
	}

	*val = pca9538_get_register(&pca_state, pca_state.reg_ptr);
	return 0;
}

static int pca9538_read_processed(struct i2c_target_config *cfg, uint8_t *val)
{
	ARG_UNUSED(cfg);

	pca_state.reg_ptr = (pca_state.reg_ptr + 1U) % PCA9538_REG_COUNT;
	*val = pca9538_get_register(&pca_state, pca_state.reg_ptr);
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
	if (!device_is_ready(i2c_dev)) {
		return 0;
	}

	pca_state.regs[PCA9538_REG_OUTPUT] = 0U;
	pca_state.regs[PCA9538_REG_POLARITY] = 0U;
	pca_state.regs[PCA9538_REG_CONFIG] = PCA9538_RESERVED_MASK;

	if (pca9538_apply_gpio_config(&pca_state) != 0) {
		return 0;
	}

	if (i2c_target_register(i2c_dev, &pca9538_target_cfg) != 0) {
		return 0;
	}

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
