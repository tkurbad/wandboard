
#include <asm-generic/errno-base.h>

#include <linux/device.h>
#include <linux/edm.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/module.h>

char *edm_expansion = NULL;
EXPORT_SYMBOL_GPL(edm_expansion);
char *edm_baseboard = NULL;
EXPORT_SYMBOL_GPL(edm_baseboard);


int edm_external_gpio[((EDM_N_EXTERNAL_GPIO) < 10 ? 10 : (EDM_N_EXTERNAL_GPIO))] = {
	[0] = -EINVAL,
	[1] = -EINVAL,
	[2] = -EINVAL,
	[3] = -EINVAL,
	[4] = -EINVAL,
	[5] = -EINVAL,
	[6] = -EINVAL,
	[7] = -EINVAL,
	[8] = -EINVAL,
	[9] = -EINVAL,
};
EXPORT_SYMBOL_GPL(edm_external_gpio);

int edm_i2c[EDM_NOF_I2C] = { -EINVAL, -EINVAL, -EINVAL };
EXPORT_SYMBOL_GPL(edm_i2c);

int edm_ddc = -EINVAL;
EXPORT_SYMBOL_GPL(edm_ddc);

int edm_spi[EDM_NOF_SPI] = { -EINVAL, -EINVAL };
EXPORT_SYMBOL_GPL(edm_spi);

struct edm_audio_data_t edm_audio_data[EDM_NOF_I2S];
EXPORT_SYMBOL_GPL(edm_audio_data);

const char *edm_gpio_pin_name[] = {
	"pin255", "pin256", "pin257", "pin258", "pin259",
	"pin260", "pin261", "pin262", "pin263", "pin264"
};

int __init edm_setup_external_gpio(void)
{
	struct class *edm_class = class_create(THIS_MODULE, "edm");
	struct device *edm_gpio;
	char *name = "external_gpio0";
	int i;

	if (IS_ERR(edm_class)) {
		pr_err("Failed to create sysfs class edm\n");
		return -ENOMEM;
	}
	edm_gpio = device_create(edm_class, NULL, 0, NULL, "gpio");
	for (i = 0; i < EDM_N_EXTERNAL_GPIO; i++) {
		if (edm_external_gpio[i] >= 0) {
			name[14] = '0' + i;
			gpio_request(edm_external_gpio[i], name);
			gpio_export(edm_external_gpio[i], true);
			gpio_export_link(edm_gpio, edm_gpio_pin_name[i], edm_external_gpio[i]);
		}
	}
	return 0;
}

static int __init edm_init(void)
{
	return edm_setup_external_gpio();
}

static void __init edm_exit(void)
{
	int i;
	for (i = 0; i < EDM_N_EXTERNAL_GPIO; i++)
		gpio_free(edm_external_gpio[i]);
}

arch_initcall_sync(edm_init);
module_exit(edm_exit);
MODULE_AUTHOR("Tapani Utriainen <tapani@technexion.com>");
MODULE_DESCRIPTION("EDM SoM <-> baseboard connection driver");
MODULE_ALIAS("EDM");
MODULE_LICENSE("GPL v2");
