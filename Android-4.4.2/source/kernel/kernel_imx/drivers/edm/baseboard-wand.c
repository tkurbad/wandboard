
#include <asm/mach/arch.h>

#include <linux/clk.h>
#include <linux/edm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>

#include <mach/common.h>
#include <mach/devices-common.h>


/****************************************************************************
 *
 * SGTL5000 Audio Codec
 *
 ****************************************************************************/

static struct regulator_consumer_supply wandbase_sgtl5000_consumer_vdda = {
	.supply = "VDDA",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_consumer_supply wandbase_sgtl5000_consumer_vddio = {
	.supply = "VDDIO",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data wandbase_sgtl5000_vdda_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &wandbase_sgtl5000_consumer_vdda,
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data wandbase_sgtl5000_vddio_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &wandbase_sgtl5000_consumer_vddio,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config wandbase_sgtl5000_vdda_reg_config = {
	.supply_name		= "VDDA",
	.microvolts		= 2500000,
	.gpio			= -1,
	.init_data		= &wandbase_sgtl5000_vdda_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config wandbase_sgtl5000_vddio_reg_config = {
	.supply_name		= "VDDIO",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &wandbase_sgtl5000_vddio_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct platform_device wandbase_sgtl5000_vdda_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 0,
	.dev	= {
		.platform_data = &wandbase_sgtl5000_vdda_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device wandbase_sgtl5000_vddio_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 1,
	.dev	= {
		.platform_data = &wandbase_sgtl5000_vddio_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device wandbase_audio_device = {
	.name = "imx-sgtl5000",
};

/* ------------------------------------------------------------------------ */

static const struct i2c_board_info wandbase_sgtl5000_i2c_data __initdata = {
	I2C_BOARD_INFO("sgtl5000", 0x0a)
};

/* ------------------------------------------------------------------------ */

static char wandbase_sgtl5000_dev_name[8] = "0-000a";

static __init int wandbase_init_sgtl5000(void)
{
	int ret = -ENODEV, i2c_bus = 1;

	/* Wand baseboard uses primary I2S, see if module provides audio and i2c */
	if (edm_audio_data[0].enabled && edm_i2c[i2c_bus] >= 0) {
		edm_audio_data[0].bus_type = EDM_BUS_I2C;
		edm_audio_data[0].bus_number = edm_i2c[i2c_bus];
		edm_audio_data[0].bus_address = 0x000a;

		sprintf(wandbase_sgtl5000_dev_name, "%1x-%04x",
			edm_audio_data[0].bus_number, edm_audio_data[0].bus_address);

		wandbase_sgtl5000_dev_name[0] = '0' + edm_audio_data[0].bus_number;
		wandbase_sgtl5000_consumer_vdda.dev_name = wandbase_sgtl5000_dev_name;
		wandbase_sgtl5000_consumer_vddio.dev_name = wandbase_sgtl5000_dev_name;

		wandbase_audio_device.dev.platform_data = edm_audio_data[0].platform_data;
		platform_device_register(&wandbase_audio_device);

		i2c_register_board_info(edm_audio_data[0].bus_number, &wandbase_sgtl5000_i2c_data, 1);
		platform_device_register(&wandbase_sgtl5000_vdda_reg_devices);
		platform_device_register(&wandbase_sgtl5000_vddio_reg_devices);
		ret = 0;
	}
	return ret;
}

/****************************************************************************
 *
 * main-function for wand baseboard
 *
 ****************************************************************************/

static __init int wandbase_init(void)
{
	if (edm_baseboard == NULL)
		return 0;
	else if (strncmp(edm_baseboard, "wand", strlen("wand")))
		return 0;

	return wandbase_init_sgtl5000();
}
arch_initcall_sync(wandbase_init);

static __exit void wandbase_exit(void)
{
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
}
module_exit(wandbase_exit);

MODULE_DESCRIPTION("Wand baseboard driver");
MODULE_AUTHOR("Tapani <tapani@vmail.me>");
MODULE_LICENSE("GPL");
