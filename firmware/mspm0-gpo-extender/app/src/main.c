#include <zephyr/kernel.h>

#include <ti/driverlib/dl_gpio.h>

#define MUX_PORT GPIOA

struct mux_output {
	uint32_t pincm;
	uint32_t pin;
	uint8_t level;
};

enum mux_selector {
	RST_WRD_SEL,
	PWM_BIT_SEL,
	AN_DI_SEL,
	INT_DO_SEL,
	CIPO_CNT_SEL0,
	CIPO_CNT_SEL1,
	MUX_SELECTOR_COUNT,
};

/*
 * Hold the muxes in a known state where AN is routed to host pin 33 and only
 * INT_DO_SEL is toggled. With AN->INT shorted, host continuity should switch
 * between pins 36 and 40 if PA10 is really moving.
 */
static const struct mux_output mux_outputs[MUX_SELECTOR_COUNT] = {
	[RST_WRD_SEL] = {
		.pincm = IOMUX_PINCM4,
		.pin = 3,
		.level = 0,
	},
	[PWM_BIT_SEL] = {
		.pincm = IOMUX_PINCM5,
		.pin = 4,
		.level = 0,
	},
	[AN_DI_SEL] = {
		.pincm = IOMUX_PINCM10,
		.pin = 9,
		.level = 1,
	},
	[INT_DO_SEL] = {
		.pincm = IOMUX_PINCM11,
		.pin = 10,
		.level = 1,
	},
	[CIPO_CNT_SEL0] = {
		.pincm = IOMUX_PINCM12,
		.pin = 11,
		.level = 1,
	},
	[CIPO_CNT_SEL1] = {
		.pincm = IOMUX_PINCM16,
		.pin = 15,
		.level = 0,
	},
};

static void set_mux_pin(const struct mux_output *output, bool level)
{
	uint32_t pin_mask = BIT(output->pin);

	if (level) {
		DL_GPIO_setPins(MUX_PORT, pin_mask);
	} else {
		DL_GPIO_clearPins(MUX_PORT, pin_mask);
	}
}

static void configure_mux_output(const struct mux_output *output)
{
	uint32_t pin_mask = BIT(output->pin);

	DL_GPIO_initDigitalOutputFeatures(output->pincm,
					  DL_GPIO_INVERSION_DISABLE,
					  DL_GPIO_RESISTOR_NONE,
					  DL_GPIO_DRIVE_STRENGTH_LOW,
					  DL_GPIO_HIZ_DISABLE);
	set_mux_pin(output, output->level != 0U);
	DL_GPIO_enableOutput(MUX_PORT, pin_mask);
}

static void configure_mux_outputs(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(mux_outputs); ++i) {
		configure_mux_output(&mux_outputs[i]);
	}
}

int main(void)
{
	bool int_on_pin_36 = true;

	configure_mux_outputs();

	while (true) {
		k_sleep(K_SECONDS(2));
		int_on_pin_36 = !int_on_pin_36;
		set_mux_pin(&mux_outputs[INT_DO_SEL], int_on_pin_36);
	}

	return 0;
}
