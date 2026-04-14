#include <zephyr/fatal.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <ti/driverlib/m0p/dl_sysctl.h>
#include <ti/driverlib/dl_gpio.h>

#define DEBUG_PINS      (BIT(19) | BIT(20))
#define DEBUG_PA19_CM   IOMUX_PINCM20
#define DEBUG_PA20_CM   IOMUX_PINCM21

static void configure_debug_gpio(void)
{
	DL_SYSCTL_disableSWD();
	DL_GPIO_initDigitalOutput(DEBUG_PA19_CM);
	DL_GPIO_initDigitalOutput(DEBUG_PA20_CM);
	DL_GPIO_enableOutput(GPIOA, DEBUG_PINS);
}

static void set_debug_pattern(bool pa19_high, bool pa20_high)
{
	configure_debug_gpio();
	DL_GPIO_clearPins(GPIOA, DEBUG_PINS);

	if (pa19_high) {
		DL_GPIO_setPins(GPIOA, BIT(19));
	}

	if (pa20_high) {
		DL_GPIO_setPins(GPIOA, BIT(20));
	}
}

static int mark_pre_kernel(void)
{
	set_debug_pattern(false, false);
	return 0;
}

SYS_INIT(mark_pre_kernel, PRE_KERNEL_1, 0);

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	ARG_UNUSED(reason);
	ARG_UNUSED(esf);

	set_debug_pattern(true, true);

	while (true) {
	}
}

int main(void)
{
	set_debug_pattern(false, true);

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
