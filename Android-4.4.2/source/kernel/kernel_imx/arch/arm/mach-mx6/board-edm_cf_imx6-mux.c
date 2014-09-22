/*
    Wandboard pin-mux file.
    Copyright (C) 2013 Edward Lin

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

#include "board-edm_cf_imx6.h"
#include <linux/edm.h>

/****************************************************************************
 *
 * SD init
 *
 * SD1 is routed to EDM connector (external SD on edm_cf_imx6 baseboard)
 * SD2 is WiFi
 * SD3 is boot SD on the module
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_sdmmc(unsigned int port, int speed)
{
	unsigned int index;

	if (speed == 200)
		index = 2;
	else if (speed == 100)
		index = 1;
	else
		index = 0;

	switch (port) {
	case 0:
		EDM_SET_PAD(PAD_SD1_CLK__USDHC1_CLK_50MHZ_40OHM);
		EDM_SET_PAD(PAD_SD1_CMD__USDHC1_CMD_50MHZ_40OHM);
		EDM_SET_PAD(PAD_SD1_DAT0__USDHC1_DAT0_50MHZ_40OHM);
		EDM_SET_PAD(PAD_SD1_DAT1__USDHC1_DAT1_50MHZ_40OHM);
		EDM_SET_PAD(PAD_SD1_DAT2__USDHC1_DAT2_50MHZ_40OHM);
		EDM_SET_PAD(PAD_SD1_DAT3__USDHC1_DAT3_50MHZ_40OHM);
		break;
	case 1:
		EDM_SET_PAD(PAD_SD2_CLK__USDHC2_CLK);
		EDM_SET_PAD(PAD_SD2_CMD__USDHC2_CMD);
		EDM_SET_PAD(PAD_SD2_DAT0__USDHC2_DAT0);
		EDM_SET_PAD(PAD_SD2_DAT1__USDHC2_DAT1);
		EDM_SET_PAD(PAD_SD2_DAT2__USDHC2_DAT2);
		EDM_SET_PAD(PAD_SD2_DAT3__USDHC2_DAT3);
		break;
	case 2:
		if (index == 2) {
			EDM_SET_PAD(PAD_SD3_CLK__USDHC3_CLK_200MHZ);
			EDM_SET_PAD(PAD_SD3_CMD__USDHC3_CMD_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT0__USDHC3_DAT0_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT1__USDHC3_DAT1_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT2__USDHC3_DAT2_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT3__USDHC3_DAT3_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT4__USDHC3_DAT4_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT5__USDHC3_DAT5_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT6__USDHC3_DAT6_200MHZ);
			EDM_SET_PAD(PAD_SD3_DAT7__USDHC3_DAT7_200MHZ);
		} else if (index == 1) {
			EDM_SET_PAD(PAD_SD3_CLK__USDHC3_CLK_100MHZ);
			EDM_SET_PAD(PAD_SD3_CMD__USDHC3_CMD_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT0__USDHC3_DAT0_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT1__USDHC3_DAT1_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT2__USDHC3_DAT2_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT3__USDHC3_DAT3_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT4__USDHC3_DAT4_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT5__USDHC3_DAT5_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT6__USDHC3_DAT6_100MHZ);
			EDM_SET_PAD(PAD_SD3_DAT7__USDHC3_DAT7_100MHZ);
		} else {
			EDM_SET_PAD(PAD_SD3_CLK__USDHC3_CLK_50MHZ);
			EDM_SET_PAD(PAD_SD3_CMD__USDHC3_CMD_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT0__USDHC3_DAT0_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT1__USDHC3_DAT1_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT2__USDHC3_DAT2_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT3__USDHC3_DAT3_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT4__USDHC3_DAT4_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT5__USDHC3_DAT5_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT6__USDHC3_DAT6_50MHZ);
			EDM_SET_PAD(PAD_SD3_DAT7__USDHC3_DAT7_50MHZ);
		}
		break;
	default:
		break;
	}
}

/****************************************************************************
 *
 * I2C
 *
 ****************************************************************************/

void edm_cf_imx6_mux_pads_init_i2c(int port)
{
	switch (port) {
	case 0:
		EDM_SET_PAD(PAD_EIM_D21__I2C1_SCL);
		EDM_SET_PAD(PAD_EIM_D28__I2C1_SDA);
		break;
	case 1:
		EDM_SET_PAD(PAD_KEY_COL3__I2C2_SCL);
		EDM_SET_PAD(PAD_KEY_ROW3__I2C2_SDA);
		break;
	case 2:
		EDM_SET_PAD(PAD_GPIO_5__I2C3_SCL);
		EDM_SET_PAD(PAD_GPIO_16__I2C3_SDA);
		break;
	default:
		printk("Invalid Port Selection : %d\n", port);
	}
}
		

/****************************************************************************
 *
 * Initialize debug console (UART1)
 *
 ****************************************************************************/

void edm_cf_imx6_mux_pads_init_uart(void)
{
	/* UART1 (debug console) */
	EDM_SET_PAD(PAD_CSI0_DAT10__UART1_TXD);
	EDM_SET_PAD(PAD_CSI0_DAT11__UART1_RXD);
	EDM_SET_PAD(PAD_EIM_D19__UART1_CTS);
	EDM_SET_PAD(PAD_EIM_D20__UART1_RTS);

	/* UART 2 */
	EDM_SET_PAD(PAD_SD4_DAT4__UART2_RXD);
	EDM_SET_PAD(PAD_SD4_DAT5__UART2_RTS);
	EDM_SET_PAD(PAD_SD4_DAT6__UART2_CTS);
	EDM_SET_PAD(PAD_SD4_DAT7__UART2_TXD);
}
	

/****************************************************************************
 *
 * Initialize sound (SSI, ASRC, AUD3 channel and S/PDIF)
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_audio(void)
{
	EDM_SET_PAD(PAD_CSI0_DAT4__AUDMUX_AUD3_TXC);
	EDM_SET_PAD(PAD_CSI0_DAT5__AUDMUX_AUD3_TXD);
	EDM_SET_PAD(PAD_CSI0_DAT6__AUDMUX_AUD3_TXFS);
	EDM_SET_PAD(PAD_CSI0_DAT7__AUDMUX_AUD3_RXD);
	EDM_SET_PAD(PAD_GPIO_0__CCM_CLKO);
}

/*****************************************************************************
 *
 * Init FEC and AR8031 PHY
 *
 *****************************************************************************/
void edm_cf_imx6_mux_pads_init_ethernet(void)
{
	EDM_SET_PAD(PAD_ENET_MDIO__ENET_MDIO);
	EDM_SET_PAD(PAD_ENET_MDC__ENET_MDC);

	EDM_SET_PAD(PAD_ENET_REF_CLK__ENET_TX_CLK);

	EDM_SET_PAD(PAD_RGMII_TXC__ENET_RGMII_TXC);
	EDM_SET_PAD(PAD_RGMII_TD0__ENET_RGMII_TD0);
	EDM_SET_PAD(PAD_RGMII_TD1__ENET_RGMII_TD1);
	EDM_SET_PAD(PAD_RGMII_TD2__ENET_RGMII_TD2);
	EDM_SET_PAD(PAD_RGMII_TD3__ENET_RGMII_TD3);
	EDM_SET_PAD(PAD_RGMII_TX_CTL__ENET_RGMII_TX_CTL);
	EDM_SET_PAD(PAD_RGMII_RXC__ENET_RGMII_RXC);
	EDM_SET_PAD(PAD_RGMII_RD0__ENET_RGMII_RD0);
	EDM_SET_PAD(PAD_RGMII_RD1__ENET_RGMII_RD1);
	EDM_SET_PAD(PAD_RGMII_RD2__ENET_RGMII_RD2);
	EDM_SET_PAD(PAD_RGMII_RD3__ENET_RGMII_RD3);
	EDM_SET_PAD(PAD_RGMII_RX_CTL__ENET_RGMII_RX_CTL);

	EDM_SET_PAD(PAD_ENET_TX_EN__GPIO_1_28);
	EDM_SET_PAD(PAD_EIM_D29__GPIO_3_29);
}

/****************************************************************************
 *
 * USB
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_usb(void)
{
	EDM_SET_PAD(PAD_GPIO_9__GPIO_1_9);
	EDM_SET_PAD(PAD_GPIO_1__USBOTG_ID);
	EDM_SET_PAD(PAD_EIM_D22__GPIO_3_22);
	EDM_SET_PAD(PAD_EIM_D30__GPIO_3_30);
}       

/****************************************************************************
 *
 * WiFi
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_wifi(void)
{
	/* bt/wifi power, signal to FDC6331L */
	EDM_SET_PAD( PAD_CSI0_DAT13__GPIO_5_31 );        
	/* wl reset */
	EDM_SET_PAD( PAD_CSI0_DAT14__GPIO_6_0 );
	/* wifi on*/
	EDM_SET_PAD( PAD_ENET_RXD1__GPIO_1_26 );
	/* host wake */
	EDM_SET_PAD( PAD_ENET_TXD1__GPIO_1_29 );
}
	

/****************************************************************************
 *
 * Bluetooth
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_bluetooth(void)
{
	/* BT_REGON, BT_RSTN, BT_WAKE and BT_HOST_WAKE */
	EDM_SET_PAD(PAD_CSI0_DATA_EN__GPIO_5_20);
	EDM_SET_PAD(PAD_CSI0_VSYNC__GPIO_5_21);
	EDM_SET_PAD(PAD_ENET_TXD0__GPIO_1_30);
	EDM_SET_PAD(PAD_CSI0_DAT12__GPIO_5_30);

	/* AUD5 channel goes to BT */
	EDM_SET_PAD(PAD_KEY_COL0__AUDMUX_AUD5_TXC);
	EDM_SET_PAD(PAD_KEY_ROW0__AUDMUX_AUD5_TXD);
	EDM_SET_PAD(PAD_KEY_COL1__AUDMUX_AUD5_TXFS);
	EDM_SET_PAD(PAD_KEY_ROW1__AUDMUX_AUD5_RXD);

	/* Bluetooth is on UART3*/
	EDM_SET_PAD(PAD_EIM_D23__UART3_CTS);
	EDM_SET_PAD(PAD_EIM_D24__UART3_TXD);
	EDM_SET_PAD(PAD_EIM_D25__UART3_RXD);
	EDM_SET_PAD(PAD_EIM_EB3__UART3_RTS);
}

/****************************************************************************
 *
 * Expansion pin header GPIOs
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_external_gpios(void)
{
	EDM_SET_PAD(PAD_CSI0_DAT19__GPIO_6_5);
	EDM_SET_PAD(PAD_CSI0_DAT16__GPIO_6_2);
	EDM_SET_PAD(PAD_CSI0_DAT18__GPIO_6_4);
	EDM_SET_PAD(PAD_EIM_D27__GPIO_3_27);
	EDM_SET_PAD(PAD_EIM_D26__GPIO_3_26);
	EDM_SET_PAD(PAD_EIM_BCLK__GPIO_6_31);
	EDM_SET_PAD(PAD_CSI0_DAT17__GPIO_6_3);
	EDM_SET_PAD(PAD_ENET_RX_ER__GPIO_1_24);
	EDM_SET_PAD(PAD_GPIO_19__GPIO_4_5);
	EDM_SET_PAD(PAD_SD3_RST__GPIO_7_8);
}

#if defined(CONFIG_EDM)
void edm_cf_imx6_external_gpios_to_edm_gpios(void)
{
	/* Associate EDM-CF-IMX6 Specific to EDM Structure*/
	edm_external_gpio[0] = IMX_GPIO_NR(6, 5); /* P255 */ /*CSI0_DAT19*/
	edm_external_gpio[1] = IMX_GPIO_NR(6, 2); /* P256 */ /*CSI0_DAT16*/
	edm_external_gpio[2] = IMX_GPIO_NR(6, 4); /* P257 */ /*CSI0_DAT18*/
	edm_external_gpio[3] = IMX_GPIO_NR(3, 27); /* P258 */ /*EIM_D27*/
	edm_external_gpio[4] = IMX_GPIO_NR(3, 26); /* P259 */ /*EIM_D26*/
	edm_external_gpio[5] = IMX_GPIO_NR(6, 31); /* P260 */ /*EIM_BCLK*/
	edm_external_gpio[6] = IMX_GPIO_NR(6, 3);  /* P261 */ /*CSI0_DAT17*/
	edm_external_gpio[7] = IMX_GPIO_NR(1, 24); /* P262 */ /*ENET_RX_ER*/
	edm_external_gpio[8] = IMX_GPIO_NR(4, 5);  /* P263 */ /*GPIO_19*/
	edm_external_gpio[9] = IMX_GPIO_NR(7, 8);  /* P264 */ /*SD3_RST*/
}
#endif

/****************************************************************************
 *
 * SPI - while not used on the Wandboard, the pins are routed out
 *
 ****************************************************************************/
void edm_cf_imx6_mux_pads_init_spi(void)
{
	EDM_SET_PAD(PAD_EIM_D16__ECSPI1_SCLK);
	EDM_SET_PAD(PAD_EIM_D17__ECSPI1_MISO);
	EDM_SET_PAD(PAD_EIM_D18__ECSPI1_MOSI);
	EDM_SET_PAD(PAD_EIM_EB2__GPIO_2_30);

	EDM_SET_PAD(PAD_EIM_CS0__ECSPI2_SCLK);
	EDM_SET_PAD(PAD_EIM_CS1__ECSPI2_MOSI);
	EDM_SET_PAD(PAD_EIM_OE__ECSPI2_MISO);
	EDM_SET_PAD(PAD_EIM_RW__GPIO_2_26);
	EDM_SET_PAD(PAD_EIM_LBA__GPIO_2_27);
}

/****************************************************************************
 *
 * LCDIF - DISPLAY PARALLEL INTERFACE
 *
 ****************************************************************************/

void edm_cf_imx6_mux_pads_init_ipu1_lcd0(void)
{
	EDM_SET_PAD(PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK);
	EDM_SET_PAD(PAD_DI0_PIN2__IPU1_DI0_PIN2);		// HSync 
	EDM_SET_PAD(PAD_DI0_PIN3__IPU1_DI0_PIN3);		// VSync 
	EDM_SET_PAD(PAD_DI0_PIN4__IPU1_DI0_PIN4);		// Contrast 
	EDM_SET_PAD(PAD_DI0_PIN15__IPU1_DI0_PIN15);		// DISP0_DRDY 
	EDM_SET_PAD(PAD_DISP0_DAT0__IPU1_DISP0_DAT_0);
	EDM_SET_PAD(PAD_DISP0_DAT1__IPU1_DISP0_DAT_1);
	EDM_SET_PAD(PAD_DISP0_DAT2__IPU1_DISP0_DAT_2);
	EDM_SET_PAD(PAD_DISP0_DAT3__IPU1_DISP0_DAT_3);
	EDM_SET_PAD(PAD_DISP0_DAT4__IPU1_DISP0_DAT_4);
	EDM_SET_PAD(PAD_DISP0_DAT5__IPU1_DISP0_DAT_5);
	EDM_SET_PAD(PAD_DISP0_DAT6__IPU1_DISP0_DAT_6);
	EDM_SET_PAD(PAD_DISP0_DAT7__IPU1_DISP0_DAT_7);
	EDM_SET_PAD(PAD_DISP0_DAT8__IPU1_DISP0_DAT_8);
	EDM_SET_PAD(PAD_DISP0_DAT9__IPU1_DISP0_DAT_9);
	EDM_SET_PAD(PAD_DISP0_DAT10__IPU1_DISP0_DAT_10);
	EDM_SET_PAD(PAD_DISP0_DAT11__IPU1_DISP0_DAT_11);
	EDM_SET_PAD(PAD_DISP0_DAT12__IPU1_DISP0_DAT_12);
	EDM_SET_PAD(PAD_DISP0_DAT13__IPU1_DISP0_DAT_13);
	EDM_SET_PAD(PAD_DISP0_DAT14__IPU1_DISP0_DAT_14);
	EDM_SET_PAD(PAD_DISP0_DAT15__IPU1_DISP0_DAT_15);
	EDM_SET_PAD(PAD_DISP0_DAT16__IPU1_DISP0_DAT_16);
	EDM_SET_PAD(PAD_DISP0_DAT17__IPU1_DISP0_DAT_17);
	EDM_SET_PAD(PAD_DISP0_DAT18__IPU1_DISP0_DAT_18);
	EDM_SET_PAD(PAD_DISP0_DAT19__IPU1_DISP0_DAT_19);
	EDM_SET_PAD(PAD_DISP0_DAT20__IPU1_DISP0_DAT_20);
	EDM_SET_PAD(PAD_DISP0_DAT21__IPU1_DISP0_DAT_21);
	EDM_SET_PAD(PAD_DISP0_DAT22__IPU1_DISP0_DAT_22);
	EDM_SET_PAD(PAD_DISP0_DAT23__IPU1_DISP0_DAT_23);
}

void edm_cf_imx6_mux_pads_init_ipu2_lcd0(void)
{
	if (!cpu_is_mx6q())
		return;

	mxc_iomux_v3_setup_pad(MX6Q_PAD_DI0_DISP_CLK__IPU2_DI0_DISP_CLK);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DI0_PIN2__IPU2_DI0_PIN2);		// HSync 
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DI0_PIN3__IPU2_DI0_PIN3);		// VSync 
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DI0_PIN4__IPU2_DI0_PIN4);		// Contrast 
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DI0_PIN15__IPU2_DI0_PIN15);		// DISP0_DRDY 
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT0__IPU2_DISP0_DAT_0);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT1__IPU2_DISP0_DAT_1);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT2__IPU2_DISP0_DAT_2);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT3__IPU2_DISP0_DAT_3);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT4__IPU2_DISP0_DAT_4);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT5__IPU2_DISP0_DAT_5);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT6__IPU2_DISP0_DAT_6);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT7__IPU2_DISP0_DAT_7);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT8__IPU2_DISP0_DAT_8);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT9__IPU2_DISP0_DAT_9);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT10__IPU2_DISP0_DAT_10);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT11__IPU2_DISP0_DAT_11);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT12__IPU2_DISP0_DAT_12);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT13__IPU2_DISP0_DAT_13);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT14__IPU2_DISP0_DAT_14);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT15__IPU2_DISP0_DAT_15);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT16__IPU2_DISP0_DAT_16);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT17__IPU2_DISP0_DAT_17);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT18__IPU2_DISP0_DAT_18);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT19__IPU2_DISP0_DAT_19);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT20__IPU2_DISP0_DAT_20);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT21__IPU2_DISP0_DAT_21);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT22__IPU2_DISP0_DAT_22);
	mxc_iomux_v3_setup_pad(MX6Q_PAD_DISP0_DAT23__IPU2_DISP0_DAT_23);
}

/****************************************************************************
 *
 * LDB - LVDS Display
 *
 ****************************************************************************/

void edm_cf_imx6_mux_pads_init_lvds(void)
{
	/* LVDS 0 */
	EDM_SET_PAD(PAD_LVDS0_CLK_P__LDB_LVDS0_CLK);
	EDM_SET_PAD(PAD_LVDS0_TX0_P__LDB_LVDS0_TX0);
	EDM_SET_PAD(PAD_LVDS0_TX1_P__LDB_LVDS0_TX1);
	EDM_SET_PAD(PAD_LVDS0_TX2_P__LDB_LVDS0_TX2);
	EDM_SET_PAD(PAD_LVDS0_TX3_P__LDB_LVDS0_TX3);
	/* LVDS 1 */
	EDM_SET_PAD(PAD_LVDS1_CLK_P__LDB_LVDS1_CLK);
	EDM_SET_PAD(PAD_LVDS1_TX0_P__LDB_LVDS1_TX0);
	EDM_SET_PAD(PAD_LVDS1_TX1_P__LDB_LVDS1_TX1);
	EDM_SET_PAD(PAD_LVDS1_TX2_P__LDB_LVDS1_TX2);
	EDM_SET_PAD(PAD_LVDS1_TX3_P__LDB_LVDS1_TX3);
}


