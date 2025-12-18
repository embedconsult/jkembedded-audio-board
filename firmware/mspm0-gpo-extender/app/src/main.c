#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(jkembedded_mux, LOG_LEVEL_INF);

int main(void)
{
	BUILD_ASSERT(IS_ENABLED(CONFIG_I2C_TARGET),
		     "I2C target mode is required for PCA953x emulation");

	LOG_INF("jkembedded_mikrobus_hat_mspm0 Zephyr build smoke test");

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
