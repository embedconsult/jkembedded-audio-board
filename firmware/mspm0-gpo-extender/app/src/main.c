#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/dt-bindings/clock/mspm0_clock.h>

#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_i2c.h>

#define MSPM0_I2C_IRQS                                                                        \
	(DL_I2C_INTERRUPT_TARGET_START | DL_I2C_INTERRUPT_TARGET_STOP |                       \
	 DL_I2C_INTERRUPT_TARGET_RXFIFO_TRIGGER | DL_I2C_INTERRUPT_TARGET_RX_DONE |          \
	 DL_I2C_INTERRUPT_TARGET_TXFIFO_EMPTY | DL_I2C_INTERRUPT_TARGET_TX_DONE |            \
	 DL_I2C_INTERRUPT_TARGET_RXFIFO_OVERFLOW | DL_I2C_INTERRUPT_TARGET_TXFIFO_UNDERFLOW)

static volatile uint8_t reg_index;
static volatile bool expect_reg_index = true;

static uint8_t target_mem[16] = {
	0x4d, 0x53, 0x50, 0x4d, 0x30, 0x20, 0x49, 0x32,
	0x43, 0x20, 0x4f, 0x4b, 0x20, 0x20, 0x20, 0x0a,
};

static void i2c0_isr(const void *arg)
{
	ARG_UNUSED(arg);

	for (DL_I2C_IIDX iidx = DL_I2C_getPendingInterrupt(I2C0);
	     iidx != DL_I2C_IIDX_NO_INT;
	     iidx = DL_I2C_getPendingInterrupt(I2C0)) {
		switch (iidx) {
		case DL_I2C_IIDX_TARGET_START:
			expect_reg_index = true;
			DL_I2C_flushTargetRXFIFO(I2C0);
			DL_I2C_flushTargetTXFIFO(I2C0);
			break;
		case DL_I2C_IIDX_TARGET_RXFIFO_TRIGGER:
		case DL_I2C_IIDX_TARGET_RX_DONE: {
			uint8_t value;

			while (DL_I2C_receiveTargetDataCheck(I2C0, &value)) {
				if (expect_reg_index) {
					reg_index = value & 0x0f;
					expect_reg_index = false;
				} else {
					target_mem[reg_index & 0x0f] = value;
					reg_index = (reg_index + 1U) & 0x0f;
				}
			}
			break;
		}
		case DL_I2C_IIDX_TARGET_TXFIFO_EMPTY:
		case DL_I2C_IIDX_TARGET_TX_DONE:
			DL_I2C_transmitTargetData(I2C0, target_mem[reg_index & 0x0f]);
			reg_index = (reg_index + 1U) & 0x0f;
			break;
		case DL_I2C_IIDX_TARGET_STOP:
			expect_reg_index = true;
			break;
		case DL_I2C_IIDX_TARGET_RXFIFO_OVERFLOW:
		case DL_I2C_IIDX_TARGET_TXFIFO_UNDERFLOW:
			DL_I2C_flushTargetRXFIFO(I2C0);
			DL_I2C_flushTargetTXFIFO(I2C0);
			expect_reg_index = true;
			break;
		default:
			break;
		}
	}
}

static void configure_i2c_target(void)
{
	DL_I2C_ClockConfig clock_cfg = {
		.clockSel = MSPM0_CLOCK_PERIPH_REG_MASK(MSPM0_CLOCK_ULPCLK),
		.divideRatio = DL_I2C_CLOCK_DIVIDE_1,
	};

	DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM1, IOMUX_PINCM1_PF_I2C0_SDA,
						    DL_GPIO_INVERSION_DISABLE,
						    DL_GPIO_RESISTOR_PULL_UP,
						    DL_GPIO_HYSTERESIS_DISABLE,
						    DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM2, IOMUX_PINCM2_PF_I2C0_SCL,
						    DL_GPIO_INVERSION_DISABLE,
						    DL_GPIO_RESISTOR_PULL_UP,
						    DL_GPIO_HYSTERESIS_DISABLE,
						    DL_GPIO_WAKEUP_DISABLE);

	DL_I2C_reset(I2C0);
	DL_I2C_enablePower(I2C0);
	delay_cycles(CONFIG_MSPM0_PERIPH_STARTUP_DELAY);
	DL_I2C_setClockConfig(I2C0, &clock_cfg);
	DL_I2C_enableAnalogGlitchFilter(I2C0);
	DL_I2C_setTargetAddressingMode(I2C0, DL_I2C_TARGET_ADDRESSING_MODE_7_BIT);
	DL_I2C_setTargetOwnAddress(I2C0, 0x20);
	DL_I2C_enableTargetOwnAddress(I2C0);
	DL_I2C_setTargetRXFIFOThreshold(I2C0, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
	DL_I2C_setTargetTXFIFOThreshold(I2C0, DL_I2C_TX_FIFO_LEVEL_EMPTY);
	DL_I2C_enableTargetClockStretching(I2C0);
	DL_I2C_enableTargetTXEmptyOnTXRequest(I2C0);
	DL_I2C_enableTargetRXFullOnRXRequest(I2C0);
	DL_I2C_disableTargetACKOverride(I2C0);
	DL_I2C_disableACKOverrideOnStart(I2C0);
	DL_I2C_clearInterruptStatus(I2C0, MSPM0_I2C_IRQS);
	DL_I2C_enableInterrupt(I2C0, MSPM0_I2C_IRQS);
	DL_I2C_enableTarget(I2C0);

	IRQ_CONNECT(I2C0_INT_IRQn, 0, i2c0_isr, NULL, 0);
	irq_enable(I2C0_INT_IRQn);
}

int main(void)
{
	configure_i2c_target();

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
