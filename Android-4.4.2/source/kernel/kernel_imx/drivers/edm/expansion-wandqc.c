
#include <linux/edm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/spi/ads7846.h>
#include <linux/spi/spi.h>

#include <mach/common.h>
#include <mach/devices-common.h>
#include <mach/gpio.h>


/****************************************************************************
 *
 * Touchscreen
 *
 ****************************************************************************/

#define WANDQC_TSC2046_IRQ edm_external_gpio[8]

static int wandqc_get_tsc2046_pendown_state(void)
{
	return !gpio_get_value(WANDQC_TSC2046_IRQ);
}

static struct ads7846_platform_data wandqc_tsc2046_config = {
	.x_max              = 0x0fff,
	.y_max              = 0x0fff,
	.pressure_max       = 255,
	.get_pendown_state  = wandqc_get_tsc2046_pendown_state,
	.keep_vref_on       = 1,
	.wakeup             = true,
};

static struct spi_board_info wandqc_tsc2046_spi_data = {
	.modalias       = "ads7846",
	.bus_num        = 0, /* Modified runtime */
	.chip_select    = 0,
	.max_speed_hz   = 1500000,
	.irq            = -EINVAL,
	.platform_data  = &wandqc_tsc2046_config,
};

static struct spi_board_info wandqc_spidev_data = {
	.modalias       = "spidev",
	.bus_num        = 0, /* Modified runtime */
	.chip_select    = 0,
	.max_speed_hz   = 15000000,
};


/* ------------------------------------------------------------------------ */

static int __init wandqc_init(void)
{
	int i;
	if (edm_expansion == NULL)
		return 0;
	else if (strncmp(edm_expansion, "wandqc", strlen("wandqc")))
		return 0;

	gpio_free(WANDQC_TSC2046_IRQ);

	gpio_request(WANDQC_TSC2046_IRQ,  "WANDQC_TSC2046_IRQ");
	gpio_direction_input(WANDQC_TSC2046_IRQ);
	gpio_export(WANDQC_TSC2046_IRQ, 0);
	gpio_set_debounce(WANDQC_TSC2046_IRQ, 0xa);

	wandqc_tsc2046_spi_data.irq = gpio_to_irq(WANDQC_TSC2046_IRQ);
	wandqc_tsc2046_spi_data.bus_num = edm_spi[0];
	spi_register_board_info(&wandqc_tsc2046_spi_data, 1);

	wandqc_spidev_data.bus_num = edm_spi[0];
	spi_register_board_info(&wandqc_spidev_data, 1);

	for (i = 0; i < EDM_N_EXTERNAL_GPIO; i++)
		if (edm_external_gpio[i] != WANDQC_TSC2046_IRQ)
			gpio_direction_output(edm_external_gpio[i], 1);
	return 0;
}
arch_initcall_sync(wandqc_init);

static __exit void wandqc_exit(void)
{
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
}
module_exit(wandqc_exit);

MODULE_DESCRIPTION("WandBoard Q/C expansion board driver");
MODULE_AUTHOR("Tapani <tapani@vmail.me>");
MODULE_LICENSE("GPL");
