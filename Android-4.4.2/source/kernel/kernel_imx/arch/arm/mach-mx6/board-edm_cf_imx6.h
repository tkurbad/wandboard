/*
    Wandboard header file. 
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

#ifndef _BOARD_EDM_CF_IMX6_H
#define _BOARD_EDM_CF_IMX6_H

#include <mach/gpio.h>
#include <mach/iomux-mx6dl.h>
#include <mach/iomux-mx6q.h>

/* Syntactic sugar for pad configuration */
#define EDM_SET_PAD(p) \
        if (cpu_is_mx6q()) \
                mxc_iomux_v3_setup_pad(MX6Q_##p);\
        else \
                mxc_iomux_v3_setup_pad(MX6DL_##p)

extern void edm_cf_imx6_mux_pads_init_sdmmc(unsigned int port, int speed);
extern void edm_cf_imx6_mux_pads_init_i2c(int port);
extern void edm_cf_imx6_mux_pads_init_uart(void);
extern void edm_cf_imx6_mux_pads_init_audio(void);
extern void edm_cf_imx6_mux_pads_init_ethernet(void);
extern void edm_cf_imx6_mux_pads_init_usb(void);
extern void edm_cf_imx6_mux_pads_init_wifi(void);
extern void edm_cf_imx6_mux_pads_init_bluetooth(void);
extern void edm_cf_imx6_mux_pads_init_external_gpios(void);
extern void edm_cf_imx6_external_gpios_to_edm_gpios(void);
extern void edm_cf_imx6_mux_pads_init_spi(void);
extern void edm_cf_imx6_mux_pads_init_ipu1_lcd0(void);
extern void edm_cf_imx6_mux_pads_init_ipu2_lcd0(void);
extern void edm_cf_imx6_mux_pads_init_lvds(void);

#endif
