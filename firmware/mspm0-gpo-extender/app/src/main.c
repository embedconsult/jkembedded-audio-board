#include <zephyr/kernel.h>

#include <ti/driverlib/dl_gpio.h>

#define DEBUG_PA19 DL_GPIO_PIN_19
#define DEBUG_PA20 DL_GPIO_PIN_20
#define DEBUG_PINS (DEBUG_PA19 | DEBUG_PA20)

static void configure_debug_outputs(void)
{
	DL_GPIO_initDigitalOutput(IOMUX_PINCM20);
	DL_GPIO_initDigitalOutput(IOMUX_PINCM21);
	DL_GPIO_clearPins(GPIOA, DEBUG_PINS);
	DL_GPIO_enableOutput(GPIOA, DEBUG_PINS);

	/* Start the two scope pins in opposite states to make them easy to distinguish. */
	DL_GPIO_setPins(GPIOA, DEBUG_PA20);
}

int main(void)
{
	configure_debug_outputs();

	while (true) {
		DL_GPIO_togglePins(GPIOA, DEBUG_PINS);
		k_msleep(100);
	}

	return 0;
}
