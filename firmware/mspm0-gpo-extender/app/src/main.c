#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <ti/driverlib/dl_gpio.h>

#define MUX_PORT GPIOA

struct mux_output {
	uint32_t pincm;
	uint32_t pin;
	uint8_t level;
};

/*
 * Default to the BeagleY-AI / AM67A routing from README.md so the first flashed
 * image drives a known-good mux state immediately after reset.
 */
static const struct mux_output mux_outputs[] = {
	{
		.pincm = IOMUX_PINCM4,
		.pin = 3,
		.level = 0,
	},
	{
		.pincm = IOMUX_PINCM5,
		.pin = 4,
		.level = 0,
	},
	{
		.pincm = IOMUX_PINCM10,
		.pin = 9,
		.level = 0,
	},
	{
		.pincm = IOMUX_PINCM11,
		.pin = 10,
		.level = 0,
	},
	{
		.pincm = IOMUX_PINCM12,
		.pin = 11,
		.level = 1,
	},
	{
		.pincm = IOMUX_PINCM16,
		.pin = 15,
		.level = 0,
	},
};

static int configure_mux_outputs(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(mux_outputs); ++i) {
		const struct mux_output *output = &mux_outputs[i];
		uint32_t pin_mask = BIT(output->pin);

		DL_GPIO_initDigitalOutputFeatures(output->pincm,
						  DL_GPIO_INVERSION_DISABLE,
						  DL_GPIO_RESISTOR_NONE,
						  DL_GPIO_DRIVE_STRENGTH_LOW,
						  DL_GPIO_HIZ_DISABLE);

		if (output->level) {
			DL_GPIO_setPins(MUX_PORT, pin_mask);
		} else {
			DL_GPIO_clearPins(MUX_PORT, pin_mask);
		}

		DL_GPIO_enableOutput(MUX_PORT, pin_mask);
	}

	return 0;
}

int main(void)
{
	int ret;

	BUILD_ASSERT(IS_ENABLED(CONFIG_I2C_TARGET),
		     "I2C target mode is required for PCA953x emulation");

	ret = configure_mux_outputs();
	if (ret < 0) {
		return ret;
	}

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
