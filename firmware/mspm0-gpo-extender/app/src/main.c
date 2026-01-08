#include <zephyr/kernel.h>

int main(void)
{
	BUILD_ASSERT(IS_ENABLED(CONFIG_I2C_TARGET),
		     "I2C target mode is required for PCA953x emulation");

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
