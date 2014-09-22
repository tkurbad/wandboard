
/* Support file for FWBADAPT-7WVGA from Future Electronics
   This file is GPL.
 */

#include <linux/delay.h>
#include <linux/edm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>

#include <mach/common.h>
#include <mach/devices-common.h>
#include <mach/gpio.h>

/****************************************************************************
 *
 * PRISM Touch
 *
 ****************************************************************************/

#include <mach/gpio.h>
#include <linux/delay.h>

#define FWBADAPT_PRISM_IRQ edm_external_gpio[8]
#define FWBADAPT_PRISM_RESET edm_external_gpio[9]

static struct i2c_board_info fwbadapt_prism_i2c_data[] = {
	{
		I2C_BOARD_INFO("prism", 0x10),
		.irq	= -EINVAL,
		.flags  = I2C_CLIENT_WAKE,
	},
};

static __init int fwbadapt_init_prism(void)
{
	gpio_direction_output(FWBADAPT_PRISM_RESET, 0);
	gpio_set_value(FWBADAPT_PRISM_RESET, 0);
	mdelay(50);
	gpio_set_value(FWBADAPT_PRISM_RESET, 1);
	fwbadapt_prism_i2c_data[0].irq = gpio_to_irq(FWBADAPT_PRISM_IRQ);
	gpio_direction_input(FWBADAPT_PRISM_IRQ);
	i2c_register_board_info(edm_i2c[1], &fwbadapt_prism_i2c_data[0], 1);
	return 0;
}

/****************************************************************************
 *
 * GPIO_BUTTON
 *
 ****************************************************************************/
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button fwbadapt_gpio_buttons[] = {
	{
		.code       = KEY_POWER,
		.desc       = "btn power-key",
		.wakeup     = 1,
		.active_low = 1,
	}, {
		.code       = KEY_HOMEPAGE,
		.desc       = "btn home-key",
		.wakeup     = 0,
		.active_low = 1,
	}, {
		.code       = KEY_ESC,
		.desc       = "btn back-key",
		.wakeup     = 0,
		.active_low = 1,
	}, {
		.code       = KEY_MENU,
		.desc       = "btn menu-key",
		.wakeup     = 0,
		.active_low = 1,
	},
};

static struct gpio_keys_platform_data fwbadapt_gpio_key_info = {
	.buttons        = fwbadapt_gpio_buttons,
	.nbuttons       = ARRAY_SIZE(fwbadapt_gpio_buttons),
};

static struct platform_device fwbadapt_keys_gpio = {
	.name   = "gpio-keys",
	.id     = -1,
	.dev    = {
		.platform_data  = &fwbadapt_gpio_key_info,
	},
};

static int __init fwbadapt_gpio_keys_init(void)
{
	gpio_free(edm_external_gpio[3]);
	gpio_free(edm_external_gpio[4]);
	gpio_free(edm_external_gpio[5]);
	gpio_free(edm_external_gpio[7]);
	fwbadapt_gpio_buttons[0].gpio = edm_external_gpio[3];
	fwbadapt_gpio_buttons[1].gpio = edm_external_gpio[4];
	fwbadapt_gpio_buttons[2].gpio = edm_external_gpio[5];
	fwbadapt_gpio_buttons[3].gpio = edm_external_gpio[7];
	platform_device_register(&fwbadapt_keys_gpio);
	return 0;
}
#else
static inline int __init fwbadapt_gpio_keys_init(void) { return 0; }
#endif

static __init int fwbadapt_init(void)
{
	int ret = 0;
	if (edm_expansion == NULL)
		return 0;
	else if (strncmp(edm_expansion, "fwbadapt", strlen("fwbadapt")))
		return 0;

	ret += fwbadapt_init_prism();
	ret += fwbadapt_gpio_keys_init();
	return ret;
}
arch_initcall_sync(fwbadapt_init);

static __exit void fwbadapt_exit(void)
{
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
}
module_exit(fwbadapt_exit);

MODULE_DESCRIPTION("FWBADAPT expansion board driver");
MODULE_LICENSE("GPL");
