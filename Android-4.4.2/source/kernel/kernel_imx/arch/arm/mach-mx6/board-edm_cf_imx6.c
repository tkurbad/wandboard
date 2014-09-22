/*
    EDM-CF-iMX6 board file.
    Copyright (C) 2013 TechNexion Ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/phy.h>

#include <mach/common.h>
#include <mach/devices-common.h>
#include <mach/gpio.h>
#include <mach/iomux-v3.h>
#include <mach/mx6.h>

#include <mach/iomux-mx6dl.h>
#include <mach/iomux-mx6q.h>

#include "crm_regs.h"
#include "devices-imx6q.h"
#include "usb.h"
#include "board-edm_cf_imx6.h"

#include <linux/edm.h>

/****************************************************************************
 *                                                                          
 * DMA controller init
 *                                                                          
 ****************************************************************************/

static __init void edm_cf_imx6_init_dma(void)
{
        imx6q_add_dma();        
}


/****************************************************************************
 *                                                                          
 * SD init
 *
 * SD1 is routed to EDM connector (external SD on edm_cf_imx6 baseboard)
 * SD2 is WiFi
 * SD3 is boot SD on the module
 *                                                                          
 ****************************************************************************/

static int fs_in_sdcard = 0;

static int edm_cf_imx6_sd_speed_change(unsigned int sd, int clock)
{
	static int pad_speed[3] = { 200, 200, 200 };

	if (clock > 100000000) {                
		if (pad_speed[sd] == 200)
			return 0;
		pad_speed[sd] = 200;
		edm_cf_imx6_mux_pads_init_sdmmc(sd, 200);
	} else if (clock > 52000000) {
		if (pad_speed[sd] == 100)
			return 0;
		pad_speed[sd] = 100;
		edm_cf_imx6_mux_pads_init_sdmmc(sd, 100);
	} else {
		if (pad_speed[sd] == 50)
			return 0;
		pad_speed[sd] = 50;
		edm_cf_imx6_mux_pads_init_sdmmc(sd, 50);
	}
	return 0;
}

/* ------------------------------------------------------------------------ */

#define EDM_CF_IMX6_SD1_CD		IMX_GPIO_NR(1, 2)
#define EDM_CF_IMX6_SD1_WP		IMX_GPIO_NR(5, 26)

static const struct esdhc_platform_data edm_cf_imx6_sd_data[3] = {
	{
		.cd_gpio                = EDM_CF_IMX6_SD1_CD,
		.wp_gpio                = -EINVAL,
		.keep_power_at_suspend  = 1,
		.support_8bit           = 0,
		.platform_pad_change    = edm_cf_imx6_sd_speed_change,
		.cd_type                = ESDHC_CD_CONTROLLER,
	}, {
		.cd_gpio                = -EINVAL,
		.wp_gpio                = -EINVAL,
		.keep_power_at_suspend  = 1,
		.support_8bit           = 0,
		.always_present         = 1,
		.platform_pad_change    = edm_cf_imx6_sd_speed_change,
		.cd_type                = ESDHC_CD_PERMANENT,
	}, {
		.cd_gpio                = -EINVAL,
		.wp_gpio                = -EINVAL,
		.keep_power_at_suspend  = 1,
		.support_18v            = 1,
		.support_8bit           = 1,
		.always_present         = 1,
		.delay_line             = 0,
		.platform_pad_change    = edm_cf_imx6_sd_speed_change,
		.cd_type                = ESDHC_CD_CONTROLLER,
	}
};

/* ------------------------------------------------------------------------ */

static __init void edm_cf_imx6_init_sd(void)
{
	int i;
	/* Card Detect for SD1 */
	EDM_SET_PAD(PAD_GPIO_2__GPIO_1_2);

	if (fs_in_sdcard == 1) {
		for (i = 0; i <= 2; i++) {
			edm_cf_imx6_mux_pads_init_sdmmc(i, 50);
			imx6q_add_sdhci_usdhc_imx(i, &edm_cf_imx6_sd_data[i]);
		}
	} else {
		for (i = 2; i >= 0; i--) {
			edm_cf_imx6_mux_pads_init_sdmmc(i, 50);
			imx6q_add_sdhci_usdhc_imx(i, &edm_cf_imx6_sd_data[i]);
		}
	}
}


/****************************************************************************
 *                                                                          
 * I2C
 *                                                                          
 ****************************************************************************/

/* ------------------------------------------------------------------------ */

static struct imxi2c_platform_data edm_cf_imx6_i2c_data[] = {
	{ .bitrate	= 400000, },
	{ .bitrate	= 400000, },
	{ .bitrate	= 400000, },
};

/* ------------------------------------------------------------------------ */

static void __init edm_cf_imx6_init_i2c(void)
{
	int i;
	for (i = 0; i < 3; i++) {
		edm_cf_imx6_mux_pads_init_i2c(i);
		imx6q_add_imx_i2c(i, &edm_cf_imx6_i2c_data[i]);
	}
}


/****************************************************************************
 *                                                                          
 * Initialize UARTs
 *                                                                          
 ****************************************************************************/

static const struct imxuart_platform_data edm_cf_imx6_external_uart_data = {
	.flags      = IMXUART_HAVE_RTSCTS,
	.dma_req_tx = MX6Q_DMA_REQ_UART2_TX,
	.dma_req_rx = MX6Q_DMA_REQ_UART2_RX,
};

/* ------------------------------------------------------------------------ */

static __init void edm_cf_imx6_init_uart(void)
{
	edm_cf_imx6_mux_pads_init_uart();

	imx6q_add_imx_uart(0, NULL);
	imx6q_add_imx_uart(1, &edm_cf_imx6_external_uart_data);
}


/****************************************************************************
 *                                                                          
 * Initialize sound (SSI, ASRC, AUD3 channel and S/PDIF)
 *                                                                          
 ****************************************************************************/
/* ------------------------------------------------------------------------ */

extern struct mxc_audio_platform_data edm_cf_imx6_audio_channel_data;

/* This function is called as a callback from the audio channel data struct */
static int edm_cf_imx6_audio_clock_enable(void)
{
	struct clk *clko;
	struct clk *new_parent;
	int rate;

	clko = clk_get(NULL, "clko_clk");
	if (IS_ERR(clko)) return PTR_ERR(clko);
	
        new_parent = clk_get(NULL, "ahb");
	if (!IS_ERR(new_parent)) {
		clk_set_parent(clko, new_parent);
		clk_put(new_parent);
	}
        
	rate = clk_round_rate(clko, 16000000);
	if (rate < 8000000 || rate > 27000000) {
		pr_err("SGTL5000: mclk freq %d out of range!\n", rate);
		clk_put(clko);
		return -1;
	}

	edm_cf_imx6_audio_channel_data.sysclk = rate;
	clk_set_rate(clko, rate);
	clk_enable(clko);
        
	return 0;        
}

/* ------------------------------------------------------------------------ */

/* This struct is added by the baseboard when initializing the codec */
struct mxc_audio_platform_data edm_cf_imx6_audio_channel_data = {
	.ssi_num    = 1,
	.src_port   = 2,
	.ext_port   = 3, /* audio channel: 3=AUD3. TODO: EDM */
	.init       = edm_cf_imx6_audio_clock_enable,
	.hp_gpio    = -1,
};
EXPORT_SYMBOL_GPL(edm_cf_imx6_audio_channel_data); /* TODO: edm naming? */

/* ------------------------------------------------------------------------ */

static int edm_cf_imx6_set_spdif_clk_rate(struct clk *clk, unsigned long rate)
{
	unsigned long rate_actual;
	rate_actual = clk_round_rate(clk, rate);
	clk_set_rate(clk, rate_actual);
	return 0;
}

/* ------------------------------------------------------------------------ */

static struct mxc_spdif_platform_data edm_cf_imx6_spdif = {
	.spdif_tx           = 1,	/* enable tx */
	.spdif_rx           = 1,	/* enable rx */
	.spdif_clk_44100    = 1,    /* tx clk from spdif0_clk_root */
	.spdif_clk_48000    = 1,    /* tx clk from spdif0_clk_root */
	.spdif_div_44100    = 23,
	.spdif_div_48000    = 37,
	.spdif_div_32000    = 37,
	.spdif_rx_clk       = 0,    /* rx clk from spdif stream */
	.spdif_clk_set_rate = edm_cf_imx6_set_spdif_clk_rate,
	.spdif_clk          = NULL, /* spdif bus clk */
};

/* ------------------------------------------------------------------------ */

static struct imx_ssi_platform_data edm_cf_imx6_ssi_pdata = {
	.flags = IMX_SSI_DMA | IMX_SSI_SYN,
};

/* ------------------------------------------------------------------------ */

static struct imx_asrc_platform_data edm_cf_imx6_asrc_data = {
	.channel_bits	= 4,
	.clk_map_ver	= 2,
};

/* ------------------------------------------------------------------------ */

void __init edm_cf_imx6_init_audio(void)
{
	edm_cf_imx6_mux_pads_init_audio();

	/* Sample rate converter is added together with audio */
	edm_cf_imx6_asrc_data.asrc_core_clk = clk_get(NULL, "asrc_clk");
	edm_cf_imx6_asrc_data.asrc_audio_clk = clk_get(NULL, "asrc_serial_clk");
	imx6q_add_asrc(&edm_cf_imx6_asrc_data);

	imx6q_add_imx_ssi(1, &edm_cf_imx6_ssi_pdata);
	/* Enable SPDIF */

	EDM_SET_PAD(PAD_ENET_RXD0__SPDIF_OUT1);

	edm_cf_imx6_spdif.spdif_core_clk = clk_get_sys("mxc_spdif.0", NULL);
	clk_put(edm_cf_imx6_spdif.spdif_core_clk);
	imx6q_add_spdif(&edm_cf_imx6_spdif);                
	imx6q_add_spdif_dai();
	imx6q_add_spdif_audio_device();
}


/*****************************************************************************
 *                                                                           
 * Init FEC and AR8031 PHY
 *                                                                            
 *****************************************************************************/
#define EDM_CF_IMX6_RGMII_INT		IMX_GPIO_NR(1, 28)
#define EDM_CF_IMX6_RGMII_RST		IMX_GPIO_NR(3, 29)

static int edm_cf_imx6_fec_phy_init(struct phy_device *phydev)
{
	unsigned short val;

	/* Ar8031 phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, 0xd, 0x3);
	phy_write(phydev, 0xe, 0x805d);
	phy_write(phydev, 0xd, 0x4003);
	val = phy_read(phydev, 0xe);
	val &= ~(0x1 << 8);
	phy_write(phydev, 0xe, val);

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, 0xd, 0x7);
	phy_write(phydev, 0xe, 0x8016);
	phy_write(phydev, 0xd, 0x4007);
	val = phy_read(phydev, 0xe);

	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, 0xe, val);

	/* Introduce tx clock delay */
	phy_write(phydev, 0x1d, 0x5);
	val = phy_read(phydev, 0x1e);
	val |= 0x0100;
	phy_write(phydev, 0x1e, val);

	/*check phy power*/
	val = phy_read(phydev, 0x0);

	if (val & BMCR_PDOWN)
		phy_write(phydev, 0x0, (val & ~BMCR_PDOWN));

	return 0;
}

/* ------------------------------------------------------------------------ */

static struct fec_platform_data edm_cf_imx6_fec_data = {
	.init   = edm_cf_imx6_fec_phy_init,
	.phy    = PHY_INTERFACE_MODE_RGMII,
};

/* ------------------------------------------------------------------------ */

static __init void edm_cf_imx6_init_ethernet(void)
{
	edm_cf_imx6_mux_pads_init_ethernet();

	gpio_request(EDM_CF_IMX6_RGMII_RST, "rgmii reset");
	gpio_direction_output(EDM_CF_IMX6_RGMII_RST, 0);
#ifdef CONFIG_FEC_1588
	mxc_iomux_set_gpr_register(1, 21, 1, 1);
#endif
	msleep(10);
	gpio_set_value(EDM_CF_IMX6_RGMII_RST, 1);
	imx6_init_fec(edm_cf_imx6_fec_data);
}


/****************************************************************************
 *                                                                          
 * USB
 *                                                                          
 ****************************************************************************/
#define EDM_CF_IMX6_USB_OTG_OC		IMX_GPIO_NR(1, 9)
#define EDM_CF_IMX6_USB_OTG_PWR		IMX_GPIO_NR(3, 22)
#define EDM_CF_IMX6_USB_H1_OC		IMX_GPIO_NR(3, 30)

static void edm_cf_imx6_usbotg_vbus(bool on)
{
	gpio_set_value_cansleep(EDM_CF_IMX6_USB_OTG_PWR, !on);
}

/* ------------------------------------------------------------------------ */

static __init void edm_cf_imx6_init_usb(void)
{
	edm_cf_imx6_mux_pads_init_usb();

	gpio_request(EDM_CF_IMX6_USB_OTG_OC, "otg oc");
	gpio_direction_input(EDM_CF_IMX6_USB_OTG_OC);

	gpio_request(EDM_CF_IMX6_USB_OTG_PWR, "otg pwr");
	gpio_direction_output(EDM_CF_IMX6_USB_OTG_PWR, 0);

	imx_otg_base = MX6_IO_ADDRESS(MX6Q_USB_OTG_BASE_ADDR);
	mxc_iomux_set_gpr_register(1, 13, 1, 1);

	mx6_set_otghost_vbus_func(edm_cf_imx6_usbotg_vbus);

	gpio_request(EDM_CF_IMX6_USB_H1_OC, "usbh1 oc");
	gpio_direction_input(EDM_CF_IMX6_USB_H1_OC);
}

/****************************************************************************
 *
 * IPU
 *
 ****************************************************************************/

static struct imx_ipuv3_platform_data edm_cf_imx6_ipu_data[] = {
	{
		.rev            = 4,
		.csi_clk[0]     = "clko_clk",
		.bypass_reset   = false,
	}, {
		.rev            = 4,
		.csi_clk[0]     = "clko_clk",
		.bypass_reset   = false,
	},
};

static __init void edm_cf_imx6_init_ipu(void)
{
	imx6q_add_ipuv3(0, &edm_cf_imx6_ipu_data[0]);
	if (cpu_is_mx6q())
		imx6q_add_ipuv3(1, &edm_cf_imx6_ipu_data[1]);
}

/****************************************************************************
 *
 * MX6 DISPLAY CONTROL FRAMEWORK
 *
 ****************************************************************************/

#include "mx6_display.h"

static void edm_cf_imx6_display0_control(int onoff)
{
	if (onoff) {
		gpio_direction_output(IMX_GPIO_NR(2, 8), 1);
		gpio_direction_output(IMX_GPIO_NR(2, 9), 1);
	} else {
		gpio_direction_output(IMX_GPIO_NR(2, 8), 0);
		gpio_direction_output(IMX_GPIO_NR(2, 9), 0);
	}

}
static void edm_cf_imx6_display1_control(int onoff)
{
	if (onoff) {
		gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
		gpio_direction_output(IMX_GPIO_NR(2, 11), 1);
	} else {
		gpio_direction_output(IMX_GPIO_NR(2, 10), 0);
		gpio_direction_output(IMX_GPIO_NR(2, 11), 0);
	}
}


static void edm_cf_imx6_init_display(void)
{
	EDM_SET_PAD(PAD_SD4_DAT0__GPIO_2_8);
	EDM_SET_PAD(PAD_SD4_DAT1__GPIO_2_9);
	EDM_SET_PAD(PAD_SD4_DAT2__GPIO_2_10);
	EDM_SET_PAD(PAD_SD4_DAT3__GPIO_2_11);

	gpio_request(IMX_GPIO_NR(2, 8), "lvds0_en");
	gpio_request(IMX_GPIO_NR(2, 9), "lvds0_blt_ctrl");

	gpio_request(IMX_GPIO_NR(2, 10), "disp0_bklen");
	gpio_request(IMX_GPIO_NR(2, 11), "disp0_vdden");

	mx6_disp_ctrls = kmalloc(sizeof(struct mx6_display_controls), GFP_KERNEL);
	if (mx6_disp_ctrls != NULL) {
		mx6_disp_ctrls->hdmi_enable     = NULL;
		mx6_disp_ctrls->lvds0_enable    = edm_cf_imx6_display0_control;
		mx6_disp_ctrls->lvds1_enable    = edm_cf_imx6_display1_control;
		mx6_disp_ctrls->lcd0_enable     = edm_cf_imx6_display1_control;
		mx6_disp_ctrls->lcd1_enable     = NULL;
		mx6_disp_ctrls->dsi_enable      = NULL;
		mx6_disp_ctrls->hdmi_pads       = NULL;
		mx6_disp_ctrls->lvds0_pads      = edm_cf_imx6_mux_pads_init_lvds;
		mx6_disp_ctrls->lvds1_pads      = edm_cf_imx6_mux_pads_init_lvds;
		mx6_disp_ctrls->lcd0_ipu1_pads  = edm_cf_imx6_mux_pads_init_ipu1_lcd0;
		mx6_disp_ctrls->lcd0_ipu2_pads  = edm_cf_imx6_mux_pads_init_ipu2_lcd0;
		mx6_disp_ctrls->lcd1_ipu1_pads  = NULL;
		mx6_disp_ctrls->lcd1_ipu2_pads  = NULL;
		mx6_disp_ctrls->dsi_pads        = NULL;
		mx6_disp_ctrls->hdmi_ddc_pads_enable    = NULL;
		mx6_disp_ctrls->hdmi_ddc_pads_disable   = NULL;
	#if defined(CONFIG_EDM)
		mx6_disp_ctrls->hdmi_i2c        = edm_ddc;
	#else
		mx6_disp_ctrls->hdmi_i2c        = 0;
	#endif
		mx6_disp_ctrls->hdcp_enable     = 0;
		mx6_disp_ctrls->lvds0_i2c       = -EINVAL;
		mx6_disp_ctrls->lvds1_i2c       = -EINVAL;
		mx6_disp_ctrls->lcd0_i2c        = -EINVAL;
		mx6_disp_ctrls->lcd1_i2c        = -EINVAL;
		mx6_disp_ctrls->dsi_i2c         = -EINVAL;
	}

	/* For EDM1 , the following are supported: lcd0, hdmi0, lvds0, dsi */
	mx6_display_ch_capability_setup(1, 0, 1, 1, 0, 0, 1);
#if 0
	/* For EDM2 , the following are supported: hdmi0, lvds0, lvds1, lvdsd, dsi */
	mx6_display_ch_capability_setup(0, 0, 1, 1, 1, 1, 1);
#endif
	mx6_init_display();
}

/****************************************************************************
 *
 * LCD Backlight Control
 *
 ****************************************************************************/
#include <linux/pwm_backlight.h>
#if defined(CONFIG_BACKLIGHT_PWM) && defined(CONFIG_MXC_PWM)
static struct platform_pwm_backlight_data edm_cf_imx6_disp0_pwm_bklight_data = {
	.pwm_id         = 2,
	.max_brightness = 248,
	.dft_brightness = 248,
	.pwm_period_ns  = 50000,
};

static struct platform_pwm_backlight_data edm_cf_imx6_disp1_pwm_bklight_data = {
	.pwm_id         = 3,
	.max_brightness = 248,
	.dft_brightness = 248,
	.pwm_period_ns  = 50000,
};

static __init void edm_cf_imx6_init_lcd_backlight(void)
{
	EDM_SET_PAD(PAD_SD4_DAT1__PWM3_PWMO);
	EDM_SET_PAD(PAD_SD4_DAT2__PWM4_PWMO);
	imx6q_add_mxc_pwm(2);
	imx6q_add_mxc_pwm(3);
	imx6q_add_mxc_pwm_backlight(2, &edm_cf_imx6_disp0_pwm_bklight_data);
	imx6q_add_mxc_pwm_backlight(3, &edm_cf_imx6_disp1_pwm_bklight_data);
}
#else
static inline void edm_cf_imx6_init_lcd_backlight(void) { ; }
#endif

/* ------------------------------------------------------------------------ */


/****************************************************************************
 *                                                                          
 * WiFi
 *                                                                          
 ****************************************************************************/
#define EDM_CF_IMX6_BTWIFI_PWR_ON	IMX_GPIO_NR(5, 31)
#define EDM_CF_IMX6_WL_RST_N		IMX_GPIO_NR(6, 0)
#define EDM_CF_IMX6_WL_REG_ON		IMX_GPIO_NR(1, 26)
#define EDM_CF_IMX6_WL_HOST_WAKE	IMX_GPIO_NR(1, 29)

/* assumes SD/MMC pins are set; call after edm_cf_imx6_init_sd() */
static __init void edm_cf_imx6_init_wifi(void)
{
	edm_cf_imx6_mux_pads_init_wifi();
                
	gpio_request(EDM_CF_IMX6_BTWIFI_PWR_ON, "btwifi_pwr");
	gpio_direction_output(EDM_CF_IMX6_BTWIFI_PWR_ON, 1);
	printk("EDWARD: btwifi_pwr GPIO_5_31 is gpio%d\n",EDM_CF_IMX6_BTWIFI_PWR_ON);
	gpio_export(EDM_CF_IMX6_BTWIFI_PWR_ON, 0);
        
	gpio_request(EDM_CF_IMX6_WL_RST_N, "wl_rst_n");
	printk("EDWARD: wl_rst_n GPIO_6_0 is gpio%d\n",EDM_CF_IMX6_WL_RST_N);
	gpio_export(EDM_CF_IMX6_WL_RST_N, 0);
	gpio_request(EDM_CF_IMX6_WL_REG_ON, "wl_reg_on");
	printk("EDWARD: wl_reg_on GPIO_1_26 is gpio%d\n",EDM_CF_IMX6_WL_REG_ON);
	gpio_export(EDM_CF_IMX6_WL_REG_ON, 0);

	gpio_direction_output(EDM_CF_IMX6_WL_RST_N, 0);
	gpio_direction_output(EDM_CF_IMX6_WL_REG_ON, 0);
	msleep(11);
	gpio_set_value(EDM_CF_IMX6_WL_RST_N, 1);
	gpio_set_value(EDM_CF_IMX6_WL_REG_ON, 1);

	gpio_request(EDM_CF_IMX6_WL_HOST_WAKE, "wl_host_wake");
	gpio_direction_input(EDM_CF_IMX6_WL_HOST_WAKE);
}


/****************************************************************************
 *                                                                          
 * Bluetooth
 *                                                                          
 ****************************************************************************/
#define EDM_CF_IMX6_BT_RSTN		IMX_GPIO_NR(5, 21)
#define EDM_CF_IMX6_BT_REGON		IMX_GPIO_NR(5, 30)
#define EDM_CF_IMX6_BT_WAKE		IMX_GPIO_NR(1, 30)
#define EDM_CF_IMX6_BT_HOST_WAKE	IMX_GPIO_NR(5, 20)
/* ------------------------------------------------------------------------ */
#include <mach/imx_rfkill.h>

static int edm_cf_imx6_bt_power_change(int status)
{
	if (status) {
		printk(KERN_INFO "edm_cf_imx6_bt_on\n");
		gpio_set_value(EDM_CF_IMX6_BT_RSTN, 1);
		gpio_set_value(EDM_CF_IMX6_BT_REGON, 1);
		msleep(100);
	} else {
		printk(KERN_INFO "edm_cf_imx6_bt_off\n");
		gpio_set_value(EDM_CF_IMX6_BT_RSTN, 0);
		gpio_set_value(EDM_CF_IMX6_BT_REGON, 0);
		msleep(11);
	}
	return 0;
}

static struct platform_device edm_cf_imx6_bt_rfkill = {
	.name = "mxc_bt_rfkill",
};

static struct imx_bt_rfkill_platform_data edm_cf_imx6_bt_rfkill_data = {
	.power_change = edm_cf_imx6_bt_power_change,
};

static const struct imxuart_platform_data edm_cf_imx6_bt_uart_data = {
	.flags      = IMXUART_HAVE_RTSCTS,
	.dma_req_tx = MX6Q_DMA_REQ_UART3_TX,
	.dma_req_rx = MX6Q_DMA_REQ_UART3_RX,
};

/* This assumes wifi is initialized (chip has power) */
static __init void edm_cf_imx6_init_bluetooth(void)
{
	edm_cf_imx6_mux_pads_init_bluetooth();

	imx6q_add_imx_uart(2, &edm_cf_imx6_bt_uart_data);

	mxc_register_device(&edm_cf_imx6_bt_rfkill, &edm_cf_imx6_bt_rfkill_data);

	gpio_request(EDM_CF_IMX6_BT_REGON, "bt_regon");
	gpio_direction_output(EDM_CF_IMX6_BT_REGON, 0);
	gpio_export(EDM_CF_IMX6_BT_REGON,0);

	gpio_request(EDM_CF_IMX6_BT_RSTN, "bt_rstn");
	gpio_direction_output(EDM_CF_IMX6_BT_RSTN, 0);
	gpio_export(EDM_CF_IMX6_BT_RSTN, 0);

	msleep(11);

	gpio_set_value(EDM_CF_IMX6_BT_RSTN, 1);
	gpio_set_value(EDM_CF_IMX6_BT_REGON, 1);

	gpio_request(EDM_CF_IMX6_BT_WAKE, "bt_wake");
	gpio_direction_output(EDM_CF_IMX6_BT_WAKE, 1);

	gpio_request(EDM_CF_IMX6_BT_HOST_WAKE, "bt_host_wake");
	gpio_direction_input(EDM_CF_IMX6_BT_WAKE);
}


/****************************************************************************
 *                                                                          
 * Power and thermal management
 *                                                                          
 ****************************************************************************/

extern bool enable_wait_mode;

static const struct anatop_thermal_platform_data edm_cf_imx6_thermal = {
	.name = "anatop_thermal",
};

/* ------------------------------------------------------------------------ */
/*Both Wifi/BT go to off behavior,according to spec 5.9.1 */
static void edm_cf_imx6_suspend_enter(void)
{
	gpio_set_value(EDM_CF_IMX6_BT_RSTN, 0);
	gpio_set_value(EDM_CF_IMX6_BT_REGON, 0);
	gpio_set_value(EDM_CF_IMX6_WL_REG_ON, 0);
}

/* ------------------------------------------------------------------------ */
/*Both Wifi/BT turn on behavior,according to  spec 5.9.1 */
static void edm_cf_imx6_suspend_exit(void)
{
	gpio_set_value(EDM_CF_IMX6_BT_RSTN, 1);
	gpio_set_value(EDM_CF_IMX6_BT_REGON, 1);
	gpio_set_value(EDM_CF_IMX6_WL_REG_ON, 1);
}

/* ------------------------------------------------------------------------ */

static const struct pm_platform_data edm_cf_imx6_pm_data = {
	.name           = "imx_pm",
	.suspend_enter  = edm_cf_imx6_suspend_enter,
	.suspend_exit   = edm_cf_imx6_suspend_exit,
};

/* ------------------------------------------------------------------------ */

static const struct mxc_dvfs_platform_data edm_cf_imx6_dvfscore_data = {
	.clk1_id		= "cpu_clk",
	.clk2_id 		= "gpc_dvfs_clk",
	.gpc_cntr_offset 	= MXC_GPC_CNTR_OFFSET,
	.ccm_cdcr_offset 	= MXC_CCM_CDCR_OFFSET,
	.ccm_cacrr_offset 	= MXC_CCM_CACRR_OFFSET,
	.ccm_cdhipr_offset 	= MXC_CCM_CDHIPR_OFFSET,
	.prediv_mask 		= 0x1F800,
	.prediv_offset 		= 11,
	.prediv_val 		= 3,
	.div3ck_mask 		= 0xE0000000,
	.div3ck_offset 		= 29,
	.div3ck_val 		= 2,
	.emac_val 		= 0x08,
	.upthr_val 		= 25,
	.dnthr_val 		= 9,
	.pncthr_val 		= 33,
	.upcnt_val 		= 10,
	.dncnt_val 		= 10,
	.delay_time 		= 80,
};

/* ------------------------------------------------------------------------ */

static void edm_cf_imx6_poweroff(void)
{
	void __iomem *snvs_lpcr = MX6_IO_ADDRESS(MX6Q_SNVS_BASE_ADDR) + 0x38;
	u32 old_lpcr = readl(snvs_lpcr);
	writel(old_lpcr | 0x60, snvs_lpcr);
}

static __init void edm_cf_imx6_init_pm(void)
{
	enable_wait_mode = false;
	imx6q_add_anatop_thermal_imx(1, &edm_cf_imx6_thermal);
	imx6q_add_pm_imx(0, &edm_cf_imx6_pm_data);
	imx6q_add_dvfs_core(&edm_cf_imx6_dvfscore_data);
	imx6q_add_busfreq();
	pm_power_off = edm_cf_imx6_poweroff;
}


/****************************************************************************
 *                                                                          
 * Expansion pin header GPIOs
 *                                                                          
 ****************************************************************************/

static __init void edm_cf_imx6_init_external_gpios(void)
{
#if defined(CONFIG_EDM)
	/* GPIO export and setup is in EDM framework, see drivers/edm/edm.c */
	edm_cf_imx6_mux_pads_init_external_gpios();
#endif
}


/****************************************************************************
 *
 * SPI - while not used on the EDM_CF_IMX6, the pins are routed out
 *
 ****************************************************************************/

static const int edm_cf_imx6_spi1_chipselect[] = { IMX_GPIO_NR(2, 30) };

/* platform device */
static const struct spi_imx_master edm_cf_imx6_spi1_data = {
	.chipselect     = (int *)edm_cf_imx6_spi1_chipselect,
	.num_chipselect = ARRAY_SIZE(edm_cf_imx6_spi1_chipselect),
};

/* ------------------------------------------------------------------------ */

static const int edm_cf_imx6_spi2_chipselect[] = { IMX_GPIO_NR(2, 26), IMX_GPIO_NR(2, 27) };

static const struct spi_imx_master edm_cf_imx6_spi2_data = {
	.chipselect     = (int *)edm_cf_imx6_spi2_chipselect,
	.num_chipselect = ARRAY_SIZE(edm_cf_imx6_spi2_chipselect),
};

/* ------------------------------------------------------------------------ */

static void __init edm_cf_imx6_init_spi(void)
{
	edm_cf_imx6_mux_pads_init_spi();

	imx6q_add_ecspi(0, &edm_cf_imx6_spi1_data);
	imx6q_add_ecspi(1, &edm_cf_imx6_spi2_data);
}

/****************************************************************************
 *                                                                          
 * Vivante GPU/VPU
 *                                                                          
 ****************************************************************************/

static struct viv_gpu_platform_data edm_cf_imx6_gpu_pdata = {
	.reserved_mem_size = SZ_128M + SZ_64M - SZ_16M,
};

static __init void edm_cf_imx6_init_gpu(void)
{
	imx_add_viv_gpu(&imx6_gpu_data, &edm_cf_imx6_gpu_pdata);
	imx6q_add_vpu();
	imx6q_add_v4l2_output(0);
}

/*****************************************************************************
 *
 * CAN bus
 *
 *****************************************************************************/

#define EDM_CF_IMX6_CAN1_STBY	IMX_GPIO_NR(7, 13)
#define EDM_CF_IMX6_CAN2_STBY	IMX_GPIO_NR(1, 7)

static void edm_cf_imx6_flexcan0_switch(int enable)
{
	gpio_set_value(EDM_CF_IMX6_CAN1_STBY, !enable);
}

/* ------------------------------------------------------------------------ */

static void edm_cf_imx6_flexcan1_switch(int enable)
{
	gpio_set_value(EDM_CF_IMX6_CAN2_STBY, !enable);
}

/* ------------------------------------------------------------------------ */

static const struct flexcan_platform_data edm_cf_imx6_flexcan0_pdata = {
	.transceiver_switch = edm_cf_imx6_flexcan0_switch,        
};

/* ------------------------------------------------------------------------ */

static const struct flexcan_platform_data edm_cf_imx6_flexcan1_pdata = {
	.transceiver_switch = edm_cf_imx6_flexcan1_switch,
};

/* ------------------------------------------------------------------------ */

void __init edm_cf_imx6_init_can(void)
{
	EDM_SET_PAD(PAD_GPIO_18__GPIO_7_13); /* CAN1_STBY */
	EDM_SET_PAD(PAD_KEY_COL2__CAN1_TXCAN);
	EDM_SET_PAD(PAD_KEY_ROW2__CAN1_RXCAN);

	EDM_SET_PAD(PAD_GPIO_7__GPIO_1_7); /* CAN2_STBY */
	EDM_SET_PAD(PAD_KEY_COL4__CAN2_TXCAN);
	EDM_SET_PAD(PAD_KEY_ROW4__CAN2_RXCAN);

	gpio_request(EDM_CF_IMX6_CAN1_STBY, "can1_stby");
	gpio_direction_output(EDM_CF_IMX6_CAN1_STBY, 0);

	gpio_request(EDM_CF_IMX6_CAN2_STBY, "can2_stby");
	gpio_direction_output(EDM_CF_IMX6_CAN2_STBY, 0);

	imx6q_add_flexcan0(&edm_cf_imx6_flexcan0_pdata);
	imx6q_add_flexcan1(&edm_cf_imx6_flexcan1_pdata);
}

/*****************************************************************************
 *
 * PCI Express
 *
 *****************************************************************************/

#define EDM_CF_IMX6_PCIE_NRST		IMX_GPIO_NR(3, 31)

static const struct imx_pcie_platform_data edm_cf_imx6_pcie_data = {
	.pcie_pwr_en	= -EINVAL,
	.pcie_rst		= EDM_CF_IMX6_PCIE_NRST,
	.pcie_wake_up	= -EINVAL,
	.pcie_dis		= -EINVAL,
#ifdef CONFIG_IMX_PCIE_EP_MODE_IN_EP_RC_SYS
	.type_ep        = 1,
#else
	.type_ep        = 0,
#endif
	.pcie_power_always_on = 1,
};

/* ------------------------------------------------------------------------ */

static void __init edm_cf_imx6_init_pcie(void)
{
	EDM_SET_PAD(PAD_EIM_D31__GPIO_3_31);
	imx6q_add_pcie(&edm_cf_imx6_pcie_data);
}

/****************************************************************************
 *
 * CAAM - I.MX6 Cryptographic Acceleration
 *
 ****************************************************************************/

static int caam_enabled;

static int __init caam_setup(char *__unused)
{
	caam_enabled = 1;
	return 1;
}
early_param("caam", caam_setup);

static void __init edm_cf_imx6_init_caam(void)
{
	if (caam_enabled) {
		pr_info("CAAM loading\n");
		imx6q_add_imx_caam();
	}
}

/****************************************************************************
 *
 * AHCI - SATA
 *
 ****************************************************************************/

#if defined(CONFIG_IMX_HAVE_PLATFORM_AHCI)
#include <mach/ahci_sata.h>

static struct clk *edm_cf_imx6_sata_clk;

/* HW Initialization, if return 0, initialization is successful. */
static int edm_cf_imx6_sata_init(struct device *dev, void __iomem *addr)
{
	u32 tmpdata;
	int ret = 0;
	struct clk *clk;

	edm_cf_imx6_sata_clk = clk_get(dev, "imx_sata_clk");
	if (IS_ERR(edm_cf_imx6_sata_clk)) {
		dev_err(dev, "no sata clock.\n");
		return PTR_ERR(edm_cf_imx6_sata_clk);
	}
	ret = clk_enable(edm_cf_imx6_sata_clk);
	if (ret) {
		dev_err(dev, "can't enable sata clock.\n");
		goto put_sata_clk;
	}

	/* Set PHY Paremeters, two steps to configure the GPR13,
	 * one write for rest of parameters, mask of first write is 0x07FFFFFD,
	 * and the other one write for setting the mpll_clk_off_b
	 *.rx_eq_val_0(iomuxc_gpr13[26:24]),
	 *.los_lvl(iomuxc_gpr13[23:19]),
	 *.rx_dpll_mode_0(iomuxc_gpr13[18:16]),
	 *.sata_speed(iomuxc_gpr13[15]),
	 *.mpll_ss_en(iomuxc_gpr13[14]),
	 *.tx_atten_0(iomuxc_gpr13[13:11]),
	 *.tx_boost_0(iomuxc_gpr13[10:7]),
	 *.tx_lvl(iomuxc_gpr13[6:2]),
	 *.mpll_ck_off(iomuxc_gpr13[1]),
	 *.tx_edgerate_0(iomuxc_gpr13[0]),
	 */
	tmpdata = readl(IOMUXC_GPR13);
	writel(tmpdata & ~0x2, IOMUXC_GPR13);
	writel(((tmpdata & ~0x07FFFFFF) | 0x0593A046), IOMUXC_GPR13);

	sata_phy_cr_addr(0x7F3F, addr);
	sata_phy_cr_write(0x1, addr);
	sata_phy_cr_read(&tmpdata, addr);

	/* Get the AHB clock rate, and configure the TIMER1MS reg later */
	clk = clk_get(NULL, "ahb");
	if (IS_ERR(clk)) {
		dev_err(dev, "no ahb clock.\n");
		ret = PTR_ERR(clk);
		goto release_sata_clk;
	}
	tmpdata = clk_get_rate(clk) / 1000;
	clk_put(clk);

#ifdef CONFIG_SATA_AHCI_PLATFORM
	ret = sata_init(addr, tmpdata);
	if (ret == 0)
		return ret;
#else
	usleep_range(1000, 2000);
	/* AHCI PHY enter into PDDQ mode if the AHCI module is not enabled */
	tmpdata = readl(addr + PORT_PHY_CTL);
	writel(tmpdata | PORT_PHY_CTL_PDDQ_LOC, addr + PORT_PHY_CTL);
	pr_info("No AHCI save PWR: PDDQ %s\n", ((readl(addr + PORT_PHY_CTL)
					>> 20) & 1) ? "enabled" : "disabled");
#endif

release_sata_clk:
	/* disable SATA_PHY PLL */
	writel((readl(IOMUXC_GPR13) & ~0x2), IOMUXC_GPR13);
	clk_disable(edm_cf_imx6_sata_clk);
put_sata_clk:
	clk_put(edm_cf_imx6_sata_clk);

	return ret;
}

#ifdef CONFIG_SATA_AHCI_PLATFORM
static void edm_cf_imx6_sata_exit(struct device *dev)
{
	clk_disable(edm_cf_imx6_sata_clk);
	clk_put(edm_cf_imx6_sata_clk);
}

static struct ahci_platform_data edm_cf_imx6_sata_data = {
	.init = edm_cf_imx6_sata_init,
	.exit = edm_cf_imx6_sata_exit,
};
#endif

static __init void edm_cf_imx6_init_ahci(void)
{
	/* SATA is not supported by MX6DL/Solo */
	if (cpu_is_mx6q())
#ifdef CONFIG_SATA_AHCI_PLATFORM
		imx6q_add_ahci(0, &edm_cf_imx6_sata_data);
#else
		edm_cf_imx6_sata_init(NULL,
			(void __iomem *)ioremap(MX6Q_SATA_BASE_ADDR, SZ_4K));
#endif

}

#else /* defined(CONFIG_IMX_HAVE_PLATFORM_AHCI) */

static inline void edm_cf_imx6_init_ahci(void) { ; }

#endif /* defined(CONFIG_IMX_HAVE_PLATFORM_AHCI) */

/*****************************************************************************
 *
 * Init EDM framework
 *
 *****************************************************************************/

#if defined(CONFIG_EDM)
static void __init edm_cf_imx6_init_edm(void)
{
	/* Associate EDM_CF_IMX6 Specific to EDM Structure*/
	edm_cf_imx6_external_gpios_to_edm_gpios();

	edm_i2c[0] = 0;
	edm_i2c[1] = 1;
	edm_i2c[2] = 2;

	edm_ddc = 0;

	edm_spi[0] = 0;
	edm_spi[1] = 1;

	edm_audio_data[0].enabled = true;
	edm_audio_data[0].platform_data = &edm_cf_imx6_audio_channel_data;
}
#else
static inline void edm_cf_imx6_init_edm(void) { ; };
#endif
/*****************************************************************************
 *                                                                           
 * Init clocks and early boot console                                      
 *                                                                            
 *****************************************************************************/

extern void __iomem *twd_base;

static void __init edm_cf_imx6_init_timer(void)
{
	struct clk *uart_clk;
#ifdef CONFIG_LOCAL_TIMERS
	twd_base = ioremap(LOCAL_TWD_ADDR, SZ_256);
#endif
	mx6_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("imx-uart.0", NULL);
	early_console_setup(UART1_BASE_ADDR, uart_clk);
}

/* ------------------------------------------------------------------------ */

static struct sys_timer edm_cf_imx6_timer = {
	.init = edm_cf_imx6_init_timer,
};

/*****************************************************************************
 *****************************************************************************/


#include <asm/setup.h>
#if defined(CONFIG_ION)

#include <linux/ion.h>

static struct ion_platform_data edm_cf_imx6_ion_data = {
	.nr = 1,
	.heaps = {
		{
		.id = 0,
		.type = ION_HEAP_TYPE_CARVEOUT,
		.name = "vpu_ion",
		.size = SZ_16M,
		.cacheable = 1,
		},
	},
};

static void edm_cf_imx6_init_ion(void)
{
	if (edm_cf_imx6_ion_data.heaps[0].size)
		imx6q_add_ion(0, &edm_cf_imx6_ion_data,
			sizeof(edm_cf_imx6_ion_data) + sizeof(struct ion_platform_heap));
}
#else
static inline void edm_cf_imx6_init_ion(void) {;}
#endif

static __init void edm_cf_imx6_init_wdt(void)
{
#if defined(CONFIG_IMX_HAVE_PLATFORM_IMX2_WDT) && defined(CONFIG_IMX2_WDT)
	imx6q_add_imx2_wdt(0, NULL);
#endif
}

static void __init fixup_edm_cf_imx6_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
	char *str;
	struct tag *t;
#if 0
	int i = 0;
	struct ipuv3_fb_platform_data *pdata_fb = edm_cf_imx6_fb_pdata;
#endif

	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_CMDLINE) {
		#if 0
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fbmem=");
			if (str != NULL) {
				str += 6;
				pdata_fb[i++].res_size[0] = memparse(str, &str);
				while (*str == ',' &&
					i < ARRAY_SIZE(edm_cf_imx6_fb_pdata)) {
					str++;
					pdata_fb[i++].res_size[0] = memparse(str, &str);
				}
			}
		#endif
		#if defined(CONFIG_ION)
			/* ION reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "ionmem=");
			if (str != NULL) {
				str += strlen("ionmem=");
				edm_cf_imx6_ion_data.heaps[0].size = memparse(str, &str);
			}
		#endif
		#if 0
			/* Primary framebuffer base address */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fb0base=");
			if (str != NULL) {
				str += strlen("fb0base=");
				pdata_fb[0].res_base[0] =
						simple_strtol(str, &str, 16);
			}
		#endif
			/* GPU reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpumem=");
			if (str != NULL) {
				str += strlen("gpumem=");
				edm_cf_imx6_gpu_pdata.reserved_mem_size = memparse(str, &str);
			}
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fs_sdcard=");
			if (str != NULL) {
				str += strlen("fs_sdcard=");
				fs_in_sdcard = memparse(str, &str);
			}
	#if defined(CONFIG_EDM)
			str = t->u.cmdline.cmdline;
			str = strstr(str, "expansion=");
			if (str != NULL) {
				str += strlen("expansion=");
				edm_expansion = str;
			}
			str = t->u.cmdline.cmdline;
			str = strstr(str, "baseboard=");
			if (str != NULL) {
				str += strlen("baseboard=");
				edm_baseboard = str;
			}
	#endif
			break;
		}
	}
}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource = {
	.name = "android ram console",
	.flags = IORESOURCE_MEM,
};

static struct platform_device android_ram_console = {
	.name = "ram_console",
	.num_resources = 1,
	.resource = &ram_console_resource,
};

static int __init edm_cf_imx6_init_ram_console(void)
{
	return platform_device_register(&android_ram_console);
}
#else
#define edm_cf_imx6_init_ram_console() do {} while (0)
#endif

#include <linux/memblock.h>
static void __init edm_cf_imx6_reserve(void)
{
	phys_addr_t phys;
	phys_addr_t total_mem = 0;
	int i;
	struct meminfo *mi = &meminfo;

	for (i = 0; i < mi->nr_banks; i++)
		total_mem += mi->bank[i].size;

#if 0
	int fb0_reserved = 0, fb_array_size;

	/*
	 * Reserve primary framebuffer memory if its base address
	 * is set by kernel command line.
	 */
	fb_array_size = ARRAY_SIZE(edm_cf_imx6_fb_pdata);
	if (fb_array_size > 0 && edm_cf_imx6_fb_pdata[0].res_base[0] &&
	    edm_cf_imx6_fb_pdata[0].res_size[0]) {
		if (edm_cf_imx6_fb_pdata[0].res_base[0] > SZ_2G)
			printk(KERN_INFO"UI Performance downgrade with FB phys address %x!\n",
			    edm_cf_imx6_fb_pdata[0].res_base[0]);
		memblock_reserve(edm_cf_imx6_fb_pdata[0].res_base[0],
				 edm_cf_imx6_fb_pdata[0].res_size[0]);
		memblock_remove(edm_cf_imx6_fb_pdata[0].res_base[0],
				edm_cf_imx6_fb_pdata[0].res_size[0]);
		edm_cf_imx6_fb_pdata[0].late_init = true;
		edm_cf_imx6_ipu_data[edm_cf_imx6_ldb_data.ipu_id].bypass_reset = true;
		fb0_reserved = 1;
	}
	for (i = fb0_reserved; i < fb_array_size; i++)
		if (edm_cf_imx6_fb_pdata[i].res_size[0]) {
			/* Reserve for other background buffer. */
			phys = memblock_alloc_base(edm_cf_imx6_fb_pdata[i].res_size[0],
						SZ_4K, total_mem);
			memblock_remove(phys, edm_cf_imx6_fb_pdata[i].res_size[0]);
			edm_cf_imx6_fb_pdata[i].res_base[0] = phys;
		}
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	phys = memblock_alloc_base(SZ_1M, SZ_4K, total_mem);
	printk("EDWARD :  ram console init at phys 0x%x\n",phys);
	memblock_remove(phys, SZ_1M);
	memblock_free(phys, SZ_1M);
	ram_console_resource.start = phys;
	ram_console_resource.end   = phys + SZ_1M - 1;
#endif

#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)
	if (edm_cf_imx6_gpu_pdata.reserved_mem_size) {
		printk("EDWARD : GPU_Reserved Memory equals to %d\n",edm_cf_imx6_gpu_pdata.reserved_mem_size);
			phys = memblock_alloc_base(edm_cf_imx6_gpu_pdata.reserved_mem_size,
						   SZ_4K, total_mem);
		printk("EDWARD :  gpumem init at phys 0x%x\n",phys);
		memblock_remove(phys, edm_cf_imx6_gpu_pdata.reserved_mem_size);
		edm_cf_imx6_gpu_pdata.reserved_mem_base = phys;
	}
#endif

#if defined(CONFIG_ION)
	if (edm_cf_imx6_ion_data.heaps[0].size) {
		phys = memblock_alloc(edm_cf_imx6_ion_data.heaps[0].size, SZ_4K);
		memblock_remove(phys, edm_cf_imx6_ion_data.heaps[0].size);
		edm_cf_imx6_ion_data.heaps[0].base = phys;
	}
#endif
}

/*****************************************************************************
 *                                                                           
 * BOARD INIT                                                                
 *                                                                            
 *****************************************************************************/

#include <mach/system.h>

extern char *gp_reg_id;
extern char *soc_reg_id;
extern u32 enable_ldo_mode;

static void __init edm_cf_imx6_board_init(void)
{

	if (enable_ldo_mode == LDO_MODE_BYPASSED) {
		gp_reg_id = "DUMMY_VDDCORE";
		soc_reg_id = "DUMMY_VDDSOC";
	}
	edm_cf_imx6_init_edm();
	edm_cf_imx6_init_dma();
	edm_cf_imx6_init_uart();
	edm_cf_imx6_init_ram_console();

	edm_cf_imx6_init_sd();
	edm_cf_imx6_init_i2c();
	edm_cf_imx6_init_audio();
	edm_cf_imx6_init_ethernet();
	edm_cf_imx6_init_usb();
	edm_cf_imx6_init_ipu();
	edm_cf_imx6_init_display();
	edm_cf_imx6_init_lcd_backlight();
	edm_cf_imx6_init_ion();

	edm_cf_imx6_init_wdt();
	edm_cf_imx6_init_ahci();

	edm_cf_imx6_init_wifi();
	edm_cf_imx6_init_bluetooth();
	edm_cf_imx6_init_pm();
	edm_cf_imx6_init_external_gpios();
	edm_cf_imx6_init_spi();
	edm_cf_imx6_init_gpu();
	edm_cf_imx6_init_can();
	edm_cf_imx6_init_caam();
	edm_cf_imx6_init_pcie();
}

/* ------------------------------------------------------------------------ */
        
MACHINE_START(EDM_CF_IMX6, "Freescale i.MX6 EDM Compact Form Factor")
	.boot_params    = MX6_PHYS_OFFSET + 0x100,
	.fixup          = fixup_edm_cf_imx6_board,
	.map_io         = mx6_map_io,
	.init_irq       = mx6_init_irq,
	.init_machine   = edm_cf_imx6_board_init,
	.timer          = &edm_cf_imx6_timer,
	.reserve        = edm_cf_imx6_reserve,
MACHINE_END

